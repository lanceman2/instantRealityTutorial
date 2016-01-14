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
    dtkCoord oldLoc;
  
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
    ThreadedNode(), oldLoc(NAN, NAN, NAN, NAN, NAN, NAN)
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

    dtkCoord loc;
    float location[6];
    Vec3f translation;
    Rotation rotation;
    dtkSharedMem* shm = new dtkSharedMem(sizeof(location), "teapot");
    assert(shm);
    if(!shm) return 1; // fail

    // Important: waitThread() in every loop
    // time is in millisecond.  Not so regular rate.
    while(waitThread(10))
    {
        // TODO: shm->blockingRead() YES YES YES
        // TODO: This memcpy sucks.
        if(shm->read(location))
            // shm->read() should have spewed.
            return -1; // fail
        loc.set(location);

        // TODO: convert units and tranform
        // TODO: Just moving viewpoint position for now
        if(oldLoc == loc) continue;
        oldLoc = loc;

        // TODO: add a x,y,z, scaling.

        dtkMatrix mat = dtkMatrix(loc);
        //mat.rotateHPR( 0.0f, -90.0f, 0.0f );
        
        // fill in translation
        mat.translate(&translation[0], &translation[1], &translation[2]);
        // fill in rotation
        mat.quat(&rotation[0], &rotation[1], &rotation[2], &rotation[3]);

        translationOutSlot_->push(translation);
        rotationOutSlot_->push(rotation);
    }

    delete shm;

    // Thread finished
    setState(NODE_SLEEPING);

    SPEW();  

    return 0;
}

} // namespace InstantIO
