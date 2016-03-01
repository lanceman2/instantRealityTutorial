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
    Matrix4f viewPoint_;
    float t;
  
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
    ThreadedNode(), t(0)
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
        "changing frustum view points (apexes) via DTK shared memory", viewPoint_);
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

// Thread method which gets automatically started as soon as a slot is
// connected
int DtkHead::processData()
{
    SPEW();  

    setState(NODE_RUNNING);

    assert(viewPointOutSlot_);

    // For stupid x-y dynamics.
    // We change x and y values from -max to max
#define PERIOD 15.0F // in seconds
    const float omega = 2*M_PI/PERIOD;// in Hz
    const int sleepStep = 10; // milliseconds
    const float amp = 0.8F;
    float x = 0;

    // Important: waitThread() in every loop
    // time is in millisecond.  Not so regular rate.
    while(waitThread(sleepStep))
    {
        // Circle x-y dynamics.
        t += sleepStep*0.001F; // t in seconds
        x = amp*cosf(omega*t);

        viewPoint_.setTranslationAxisAngle(Vec3f(x,0,1), Vec3f(0,1,0), 0);
        viewPointOutSlot_->push(viewPoint_);
        if(t > PERIOD)
            t -= PERIOD;
    }

    // Thread finished
    setState(NODE_SLEEPING);

    SPEW();  

    return 0;
}

} // namespace InstantIO
