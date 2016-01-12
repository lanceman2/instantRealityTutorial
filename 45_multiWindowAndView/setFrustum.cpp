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
#include <string.h>

#define __FILENAME__ \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define SPEW()   std::cerr << __FILENAME__ << ":" << __LINE__\
  << " " <<  __func__ << "()" << std::endl


namespace InstantIO
{

template <class T> class OutSlot;

class SetFrustum : public ThreadedNode
{
public:
    SetFrustum();
    virtual ~SetFrustum();

    // Factory method to create an instance of SetFrustum.
    static Node *create();
  
    // Factory method to return the type of SetFrustum.
    virtual NodeType *type() const;
  
protected:
    // Gets called when the SetFrustum is enabled.
    virtual void initialize();
  
    // Gets called when the SetFrustum is disabled.
    virtual void shutdown();
  
    // thread method to send/receive data.
    virtual int processData ();
  
private:
    
    OutSlot<Matrix4f> *viewPointOutSlot_;
    Matrix4f viewPoint_;
  
    static NodeType type_;
};


NodeType SetFrustum::type_(
    "SetFrustum" /*typeName must be the same as plugin filename */,
    &create,
    "sets view frustum" /*shortDescription*/,
    "example tutorial that sets wiew frustum with C++ code" /*longDescription*/,
    "lanceman"/*author*/,
    0/*fields*/,
    0/sizeof(Field));

SetFrustum::SetFrustum() :
    ThreadedNode()
{
    SPEW();
    // Add external route
    addExternalRoute("*", "{NamespaceLabel}/{SlotLabel}");

    SPEW();
}

SetFrustum::~SetFrustum()
{
    SPEW();
}

Node *SetFrustum::create()
{
    SPEW();  
    return new SetFrustum;
}

NodeType *SetFrustum::type() const
{
    SPEW();  
    return &type_;
}

void SetFrustum::initialize()
{
    SPEW();  
    
    // handle state and namespace updates
    Node::initialize();

    viewPointOutSlot_ = new OutSlot<Matrix4f>(
        "changing viewpoint", viewPoint_);
    assert(viewPointOutSlot_);
    viewPointOutSlot_->addListener(*this);
    addOutSlot("viewPoint", viewPointOutSlot_);
}

//----------------------------------------------------------------------
void SetFrustum::shutdown()
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

// Thread method which gets automatically started as soon as a slot is
// connected
int SetFrustum::processData()
{
    SPEW();  

    setState(NODE_RUNNING);

    assert(viewPointOutSlot_);

    // For stupid x-y dynamics.
    const float max = 0.5F, step = 0.001F;
    float y = 0, x = max/2;
    int ud = -1, rl = 1;

    // Important: waitThread() in every loop
    // time is in millisecond.  Not so regular rate.
    while(waitThread(10))
    {
        // Stupid x-y dynamics.
        x += rl*step*1.14;
        if(rl > 0 && x > max)
          rl = -1;
        else if(rl < 0 && x < -max)
          rl = 1;

        y += ud*step;
        if(ud > 0 && y > max)
          ud = -1;
        else if(ud < 0 && y < -max)
          ud = 1;

        viewPoint_.setTranslationAxisAngle(Vec3f(x,y,1), Vec3f(0,0,1), 0);
        viewPointOutSlot_->push(viewPoint_);
    }

    // Thread finished
    setState(NODE_SLEEPING);

    SPEW();  

    return 0;
}

} // namespace InstantIO
