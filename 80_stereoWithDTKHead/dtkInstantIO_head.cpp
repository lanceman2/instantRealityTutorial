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

class DtkHead : public ThreadedNode
{
public:
    DtkHead();
    virtual ~DtkHead();

    // Factory method to create an instance of DtkHead.
    static Node *create();
  
    // Factory method to return the type of DtkHead.
    virtual NodeType *type() const;
  
protected:
    // Gets called when the DtkHead is enabled.
    virtual void initialize();
  
    // Gets called when the DtkHead is disabled.
    virtual void shutdown();
  
    // thread method to send/receive data.
    virtual int processData ();
  
private:

    OutSlot<Matrix4f> *viewPointOutSlot_;
  
    static NodeType type_;
};


NodeType DtkHead::type_(
    "dtkInstantIO_head" /*typeName must be the same as plugin filename */,
    &create,
    "DTK shared memory head to InstantIO" /*shortDescription*/,
    /*longDescription*/
    "sets view frustum view point Matrix4f by reading DTK shared memory named head",
    "lanceman"/*author*/,
    0/*fields*/,
    0/sizeof(Field));

DtkHead::DtkHead() :
    ThreadedNode()
{
    SPEW();
    // Add external route
    addExternalRoute("*", "{NamespaceLabel}/{SlotLabel}");

    SPEW();
}

DtkHead::~DtkHead()
{
    SPEW();
}

Node *DtkHead::create()
{
    SPEW();  
    return new DtkHead;
}

NodeType *DtkHead::type() const
{
    SPEW();  
    return &type_;
}

void DtkHead::initialize()
{
    SPEW();  
    
    // handle state and namespace updates
    Node::initialize();

    viewPointOutSlot_ = new OutSlot<Matrix4f>(
        "changing frustum view points (apexes) via DTK shared memory", Matrix4f());
    assert(viewPointOutSlot_);
    viewPointOutSlot_->addListener(*this);
    addOutSlot("viewPoint", viewPointOutSlot_);
}

void DtkHead::shutdown()
{
    // handle state and namespace updates
    Node::shutdown();

    // ###ADDCODE###
    // remove all dynamic slots and do other cleanups
    // setState (NODE_ERROR) on error

    SPEW();  

    assert(viewPointOutSlot_);

    if(viewPointOutSlot_)
    {
        removeOutSlot("viewPoint", viewPointOutSlot_);
        delete viewPointOutSlot_;
        viewPointOutSlot_ = 0;
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

// Thread method which gets automatically started as soon as a slot is
// connected
int DtkHead::processData()
{
    SPEW();  

    setState(NODE_RUNNING);

    assert(viewPointOutSlot_);

    Matrix4f viewPoint;
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

        viewPoint.setTransform(translation, rotation);

        // send out the InstantReality transform view matrix
        viewPointOutSlot_->push(viewPoint);
    }

    delete shm;

    // Thread finished
    setState(NODE_SLEEPING);

    SPEW();  

    return 0;
}

} // namespace InstantIO
