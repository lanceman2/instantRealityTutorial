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
#include <InstantIO/Rotation.h>
#include <InstantIO/Vec3.h>

namespace InstantIO
{

template <class T> class OutSlot;

class Rotator : public ThreadedNode
{
public:
    Rotator();
    virtual ~Rotator();

    // Factory method to create an instance of Rotator.
    static Node *create();
  
    // Factory method to return the type of Rotator.
    virtual NodeType *type() const;
  
protected:
    // Gets called when the Rotator is enabled.
    virtual void initialize();
  
    // Gets called when the Rotator is disabled.
    virtual void shutdown();
  
    // thread method to send/receive data.
    virtual int processData ();
  
private:
    
    OutSlot<Rotation> *rotation_;
    float ang_;
    Vec3f axis_;
  
    static NodeType type_;
};


NodeType Rotator::type_(
    "Rotator" /*typeName must be the same as plugin filename */,
    &create,
    "rotate" /*shortDescription*/,
    "rotate via C++ code" /*longDescription*/,
    "lanceman"/*author*/,
    0/*fields*/,
    0/sizeof(Field));


Rotator::Rotator() : 
    ThreadedNode(), rotation_(0), ang_(0), axis_(0,1,0)
{
    std::cerr << __FILE__ << ": " << __func__ << "()" << std::endl;
    // Add external route
    addExternalRoute("*", "{NamespaceLabel}/{SlotLabel}");
}

Rotator::~Rotator()
{
    std::cerr << __FILE__ << ": " << __func__ << "()" << std::endl;
}

Node *Rotator::create()
{
    std::cerr << __FILE__ << ": " << __func__ << "()" << std::endl;
    return new Rotator;
}

NodeType *Rotator::type() const
{
    std::cerr << __FILE__ << ": " << __func__ << "()" << std::endl;
    return &type_;
}

void Rotator::initialize()
{
    std::cerr << __FILE__ << ": " << __func__ << "()" << std::endl;
    
    // handle state and namespace updates
    Node::initialize();

    rotation_ = new OutSlot<Rotation>("changing rotation", Rotation(0,0,1,0));
    assert(rotation_);
    rotation_->addListener(*this);
    addOutSlot("rotation", rotation_);
}

void Rotator::shutdown()
{
    // handle state and namespace updates
    Node::shutdown();

    // setState (NODE_ERROR) on error

    std::cerr << __FILE__ << ": " << __func__ << "()" << std::endl;

    assert(rotation_);

    if(rotation_)
    {
        removeOutSlot("rotation", rotation_);
        delete rotation_;
        rotation_ = 0;
    }
}

// Thread method which gets automatically started as soon as a slot is
// connected
int Rotator::processData()
{
    std::cerr << __FILE__ << ": " << __func__ << "()" << std::endl;

    setState(NODE_RUNNING);

    assert(rotation_);
    Rotation rot;

    // Important: waitThread() in every loop
    // time is in millisecond.  Shitty cheap method.
    while(waitThread(10))
    {
      // TODO: Note that this is not syncing to a realtime,
      // it's just a haphazard rotation rate, we need to add
      // a realtime wait timer and/or realtime time measure
      // to make a rotation rate that is uniform over realtime.
      //
      // It's a rotation angle in radians
      // and we keep it's value bounded.
      if(ang_ < M_PI)
        ang_ += 0.01F; // rotation angle increase per loop time
      else
        ang_ -= 2*M_PI + 0.01F;

      // Since we do not know how instantReality is defining their
      // quaternion 4-vectors we use axis and angle (we know what
      // they are).  And yes axis is constant and normalized to
      // magnitude 1. We don't know if axis needs to be normalized.
      rot.set(axis_, ang_);
      // Looks like instantReality API copies the Rotation
      // object at every loop.  Kind of inefficient if you
      // ask me.
      rotation_->push(rot);
     }

    // Thread finished
    setState(NODE_SLEEPING);

    std::cerr << __FILE__ << ": finish processData" << std::endl;

    return 0;
}

} // namespace InstantIO
