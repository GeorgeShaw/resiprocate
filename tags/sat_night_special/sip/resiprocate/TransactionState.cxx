#include <sipstack/TransactionState.hxx>
#include <sipstack/SipStack.hxx>
#include <sipstack/SipMessage.hxx>
#include <sipstack/TimerMessage.hxx>
#include <sipstack/Logger.hxx>
#include <sipstack/MethodTypes.hxx>

using namespace Vocal2;

#define VOCAL_SUBSYSTEM Subsystem::SIP

void
TransactionState::process(SipStack& stack)
{
   Message* message = stack.mStateMacFifo.getNext();
   
   SipMessage* sip = dynamic_cast<SipMessage*>(message);
   TimerMessage* timer=0;
   
   if (sip == 0)
   {
      timer = dynamic_cast<TimerMessage*>(message);
   }
   
   const Data& tid = message->getTransactionId();
   TransactionState* state = stack.mTransactionMap.find(tid);
   if (state) // found transaction for sip msg
   {
      switch (state->mMachine)
      {
         case ClientNonInvite:
            state->processClientNonInvite(message);
            break;
         case ClientInvite:
            state->processClientInvite(message);
            break;
         case ServerNonInvite:
            state->processServerNonInvite(message);
            break;
         case ServerInvite:
            state->processClientInvite(message);
            break;
         case Stale:
            state->processStale(message);
            break;
         default:
            assert(0);
      }
   }
   else // new transaction
   {
      if (sip)
      {
         if (sip->isRequest())
         {
            // create a new state object and insert in the TransactionMap
               
            if (sip->isExternal()) // new sip msg from transport
            {
               if ((*sip)[RequestLine].getMethod() == INVITE)
               {
                  TransactionState* state = new TransactionState(stack, ServerInvite, Proceeding);
                  stack.mTimers.add(Timer::TimerTrying, tid, Timer::T100);
                  stack.mTransactionMap.add(tid,state);
               }
               else 
               {
                  TransactionState* state = new TransactionState(stack, ServerNonInvite,Trying);
                  stack.mTransactionMap.add(tid,state);
               }
               stack.mTUFifo.add(sip);
            }
            else // new sip msg from the TU
            {
               if ((*sip)[RequestLine].getMethod() == INVITE)
               {
                  TransactionState* state = new TransactionState(stack, ClientInvite, Calling);
                  stack.mTimers.add(Timer::TimerB, tid, 64*Timer::T1 );
                  stack.mTransactionMap.add(tid,state);
               }
               else 
               {
                  TransactionState* state = new TransactionState(stack, ClientNonInvite, Trying);
                  stack.mTimers.add(Timer::TimerF, tid, 64*Timer::T1 );
                  stack.mTransactionMap.add(tid,state);
               }
               stack.mTransportSelector.send(sip);
            }
         }
         else if (sip->isResponse()) // stray response
         {
            if (stack.mDiscardStrayResponses)
            {
               DebugLog (<< "discarding stray response: " << sip->brief());
               delete message;
            }
            else
            {
               // forward this statelessly
               assert(0);
            }
         }
         else // wasn't a request or a response
         {
            DebugLog (<< "discarding unknown message: " << sip->brief());
         }
      } 
      else // timer or other non-sip msg
      {
         DebugLog (<< "discarding non-sip message: " << message->brief());
         delete message;
      }
   }
}

void
TransactionState::processClientNonInvite(  Message* msg )
{ 

}


void
TransactionState::processClientInvite(  Message* msg )
{
}


void
TransactionState::processServerNonInvite(  Message* msg )
{
   switch (mState)
   {
      case Trying:
         if (isFinalResponse(msg) && isFromTU(msg))
         {
            if (mIsReliable)
            {
               delete this;
            }
            else
            {
               mState = Completed;
               mStack.mTimers.add(Timer::TimerJ, msg->getTransactionId(), 64*Timer::T1 );
            }
         }
         else if (isProvisionalResponse(msg) && isFromTU(msg))
         {
            mState = Proceeding;
            mStack.mTransportSelector.send(dynamic_cast<SipMessage*>(msg));
         }
         else
         {
            assert(0);
         }
         break;

      case Proceeding:
         if (isFinalResponse(msg) && isFromTU(msg))
         {
            if (mIsReliable)
            {
               delete this;
            }
            else
            {
               mState = Completed;
               mStack.mTimers.add(Timer::TimerJ, msg->getTransactionId(), 64*Timer::T1 );
            }
         }
         else if (isProvisionalResponse(msg) && isFromTU(msg))
         {
            // retransmit
            mStack.mTransportSelector.send(dynamic_cast<SipMessage*>(msg));
         }
         else if (isRequest(msg))
         {
            mStack.mTransportSelector.send(mMsgToRetransmit);
         }
         else if (isTranportError(msg))
         {
            delete this;
         }
         else 
         {
            assert(0);
         }
         break;
         
      case Completed:
         if (TimerMessage* timer = dynamic_cast<TimerMessage*>(msg))
         {
            if (timer->getType() == Timer::TimerJ)
            {
               delete timer;
            }
         }
         else if (isRequest(msg))
         {
            mStack.mTransportSelector.send(mMsgToRetransmit);
         }
         else if (isTranportError(msg))
         {
            delete this;
         }
         else 
         {
            assert(0);
         }
         break;
   }
}


void
TransactionState::processServerInvite(  Message* msg )
{
}


void
TransactionState::processStale(  Message* msg )
{
}

bool
TransactionState::isRequest(Message* msg) const
{
   SipMessage* sip = dynamic_cast<SipMessage*>(msg);   
   return sip && sip->isRequest();
}

bool 
TransactionState::isFinalResponse(Message* msg) const
{
   SipMessage* sip = dynamic_cast<SipMessage*>(msg);
   return sip && sip->isResponse() && (*sip)[StatusLine].getResponseCode() >= 200;
}

bool 
TransactionState::isProvisionalResponse(Message* msg) const
{
   SipMessage* sip = dynamic_cast<SipMessage*>(msg);
   return sip && sip->isResponse() && (*sip)[StatusLine].getResponseCode() < 200;
}

bool 
TransactionState::isFailureResponse(Message* msg) const
{
   SipMessage* sip = dynamic_cast<SipMessage*>(msg);
   return sip && sip->isResponse() && (*sip)[StatusLine].getResponseCode() >= 300;
}

bool 
TransactionState::isSuccessResponse(Message* msg) const
{
   SipMessage* sip = dynamic_cast<SipMessage*>(msg);
   if (sip && sip->isResponse())
   {
      int c = (*sip)[StatusLine].getResponseCode() == 200; // !jf!
      return c >= 200 || c < 300;
   }
   else 
   {
      return false;
   }
}

bool
TransactionState::isFromTU(Message* msg) const
{
   SipMessage* sip = dynamic_cast<SipMessage*>(msg);
   return sip && !sip->isExternal();
}

bool
TransactionState::isTranportError(Message* msg) const
{
   return false; // !jf!
}

