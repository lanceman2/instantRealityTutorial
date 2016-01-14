//#define NDEBUG
#include <assert.h>
#include <math.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include <InstantIO/ThreadedNode.h>
#include <InstantIO/NodeType.h>
#include <InstantIO/OutSlot.h>
#include <InstantIO/MFTypes.h>
#include <InstantIO/Vec3.h>
#include <InstantIO/Matrix4.h>

#include <dtk.h>

#include <string.h>

#define __FILENAME__ \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define SPEW()   std::cerr << __FILENAME__ << ":" << __LINE__\
  << " " <<  __func__ << "()" << std::endl


namespace InstantIO
{

template <class T> class OutSlot;

class DtkShmMove : public ThreadedNode
{
public:
    DtkShmMove();
    virtual ~DtkShmMove();

    // Factory method to create an instance of DtkShmMove.
    static Node *create();
  
    // Factory method to return the type of DtkShmMove.
    virtual NodeType *type() const;
  
protected:
    // Gets called when the DtkShmMove is enabled.
    virtual void initialize();
  
    // Gets called when the DtkShmMove is disabled.
    virtual void shutdown();
  
    // thread method to send/receive data.
    virtual int processData ();
  
private:

    OutSlot<Vec3f> *translationOutSlot_;
    OutSlot<Rotation> *rotationOutSlot_;
  
    static NodeType type_;
};


NodeType DtkShmMove::type_(
    "dtkShmMove" /*typeName must be the same as plugin filename */,
    &create,
    "DTK shm position and orientation"/*shortDescription*/, 
    /*longDescription*/
    "DTK shared memory position and orientation to InstantIO",
    "lanceman"/*author*/,
    0/*fields*/, // no input from X3D
    0/sizeof(Field));

DtkShmMove::DtkShmMove() :
    ThreadedNode() 
{
    SPEW();
    // Add external route
    addExternalRoute("*", "{NamespaceLabel}/{SlotLabel}");
    SPEW();
}

DtkShmMove::~DtkShmMove()
{
    SPEW();
}

Node *DtkShmMove::create()
{
    SPEW();  
    return new DtkShmMove;
}

NodeType *DtkShmMove::type() const
{
    SPEW();  
    return &type_;
}

void DtkShmMove::initialize()
{
    SPEW();  
    
    // handle state and namespace updates
    Node::initialize();

    translationOutSlot_ = new OutSlot<Vec3f>(
        "position via DTK shared memory", Vec3f(0, 0, 0));
    assert(translationOutSlot_);
    translationOutSlot_->addListener(*this);
    addOutSlot("translation", translationOutSlot_);

    rotationOutSlot_ = new OutSlot<Rotation>(
        "rotation via DTK shared memory", Rotation(0,0,1,0));
    assert(rotationOutSlot_);
    rotationOutSlot_->addListener(*this);
    addOutSlot("rotation", rotationOutSlot_);
}

void DtkShmMove::shutdown()
{
    // handle state and namespace updates
    Node::shutdown();

    // ###ADDCODE###
    // remove all dynamic slots and do other cleanups
    // setState (NODE_ERROR) on error

    SPEW();  

    assert(translationOutSlot_);

    if(translationOutSlot_)
    {
        removeOutSlot("translation", translationOutSlot_);
        delete translationOutSlot_;
        translationOutSlot_ = 0;
    }

    assert(rotationOutSlot_);

    if(rotationOutSlot_)
    {
        removeOutSlot("rotation", rotationOutSlot_);
        delete rotationOutSlot_;
        rotationOutSlot_ = 0;
    }
}

static inline
bool IsSameOrSetFloat6(float x[6], const float y[6])
{
    int i;
    for(i=0;i<6;++i)
        if(x[i] != y[i])
            break;
    if(i == 6) return true; // they are the same
    // cannot use sizeof(x) below.
    memcpy(x, y, 6*sizeof(float));
    return false;
}

// TODO: this code sucks, too much memory copying,
// converting  to unnecessary intermediate forms
// Thread method which gets automatically started as soon as a slot is
// connected
int DtkShmMove::processData()
{
    SPEW();  

    setState(NODE_RUNNING);

    assert(translationOutSlot_);
    assert(rotationOutSlot_);

    float loc[6];
    float oldLoc[6] = { NAN, NAN, NAN, NAN, NAN, NAN };
    Vec3f translation;
    Rotation rotation;
    dtkMatrix mat;
    dtkSharedMem* shm = new dtkSharedMem(sizeof(loc), "head");
    assert(shm);
    if(!shm) return 1; // fail

    // TODO: We should be waiting at the sharedMemory read call and
    // not here.  That would give much better performance in so many
    // ways.  Problem is, we have no easy way to interrupt the blocking
    // shm->blockingRead(loc) call at quiting time.
    while(waitThread(10))
    {
       if(shm->read(loc)) // not blocking here like a good thread should not
       // if(shm->blockingRead(loc)) // blocking here like a good thread should
            // the above call should have spewed.
            return -1; // fail

        // TODO: convert units and tranform
        // TODO: Just moving viewpoint position for now
        if(IsSameOrSetFloat6(oldLoc, loc)) continue;

        // TODO: add a x,y,z, scaling.

        // fill in InstantReality translation
        translation.set(loc[0], loc[2], -loc[1]);
        // send out the InstantReality translation
        translationOutSlot_->push(translation);

        mat.identity();
        // Unless you wrote the DTK matrix code yesterday forget about
        // understanding this code without running the example in
        // InstantPlayer.

        // Through trial and error we found that we need to
        // set up these three calls in this order.
        // You cannot do this in one rotateHPR() call.
        mat.rotateHPR(-loc[5], 0, 0);// Diverse Heading is minus InstantReality Roll
        mat.rotateHPR(0, loc[4], 0); // Diverse Pitch is InstantReality Pitch
        mat.rotateHPR(0, 0, loc[3]); // Diverse Roll is InstantReality Heading

        // fill in InstantReality rotations in quaternions.
        mat.quat(&rotation[0], &rotation[1], &rotation[2], &rotation[3]);
        // send out the InstantReality rotation
        rotationOutSlot_->push(rotation);
    }

    delete shm;

    // Thread finished
    setState(NODE_SLEEPING);

    SPEW();  

    return 0;
}

} // namespace InstantIO
