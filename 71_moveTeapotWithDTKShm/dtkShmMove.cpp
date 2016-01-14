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

// Thread method which gets automatically started as soon as a slot is
// connected
int DtkShmMove::processData()
{
    SPEW();  

    setState(NODE_RUNNING);

    assert(translationOutSlot_);
    assert(rotationOutSlot_);

    float loc[6];
    Vec3f translation;
    Rotation rotation;
    dtkSharedMem* shm = new dtkSharedMem(sizeof(loc), "teapot");
    assert(shm);
    if(!shm) return 1; // fail

    // Important: waitThread() in every loop
    // time is in millisecond.  Not so regular rate.
    while(waitThread(10))
    {
        // TODO: shm->blockingRead() YES YES YES
        // TODO: This memcpy sucks.
        shm->read(loc);
        // TODO: convert units and tranform
        // TODO: Just moving viewpoint position for now 

        translation.set(loc[0], loc[1], loc[2]);
        //rotation.set(Vec3f(0,1,0), loc[3]*M_PI/180.0F);
        //rotation.set(Vec3f(1,0,0), loc[4]*M_PI/180.0F);
        rotation.set(Vec3f(0,0,-1), loc[5]*M_PI/180.0F);
        rotationOutSlot_->push(rotation);
        translationOutSlot_->push(translation);
    }

    delete shm;

    // Thread finished
    setState(NODE_SLEEPING);

    SPEW();  

    return 0;
}

} // namespace InstantIO
