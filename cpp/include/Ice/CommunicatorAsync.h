//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_COMMUNICATOR_ASYNC_H
#define ICE_COMMUNICATOR_ASYNC_H

#ifndef ICE_CPP11_MAPPING

#include <Ice/Communicator.h>

namespace Ice
{

/**
 * Type-safe asynchronous callback wrapper class used for calls to
 * Ice::Communicator::begin_flushBatchRequests.
 * Create a wrapper instance by calling ::Ice::newCallback_Communicator_flushBatchRequests.
 * \headerfile Ice/Ice.h
 */
template<class T>
class CallbackNC_Communicator_flushBatchRequests : public Callback_Communicator_flushBatchRequests_Base,
                                                   public ::IceInternal::OnewayCallbackNC<T>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);

    CallbackNC_Communicator_flushBatchRequests(const TPtr& obj, Exception excb, Sent sentcb)
        : ::IceInternal::OnewayCallbackNC<T>(obj, 0, excb, sentcb)
    {
    }

    /// \cond INTERNAL
    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        ::Ice::CommunicatorPtr communicator = result->getCommunicator();
        assert(communicator);
        try
        {
            communicator->end_flushBatchRequests(result);
            assert(false);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::CallbackNC<T>::exception(result, ex);
        }
    }
    /// \endcond
};

/**
 * Creates a callback wrapper instance that delegates to your object.
 * @param instance The callback object.
 * @param excb The exception method of the callback object.
 * @param sentcb The sent method of the callback object.
 * @return An object that can be passed to an asynchronous invocation of
 * Ice::Communicator::begin_flushBatchRequests.
 */
template<class T> Callback_Communicator_flushBatchRequestsPtr
newCallback_Communicator_flushBatchRequests(const IceUtil::Handle<T>& instance,
                                            void (T::*excb)(const ::Ice::Exception&),
                                            void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Communicator_flushBatchRequests<T>(instance, excb, sentcb);
}

/**
 * Creates a callback wrapper instance that delegates to your object.
 * @param instance The callback object.
 * @param excb The exception method of the callback object.
 * @param sentcb The sent method of the callback object.
 * @return An object that can be passed to an asynchronous invocation of
 * Ice::Communicator::begin_flushBatchRequests.
 */
template<class T> Callback_Communicator_flushBatchRequestsPtr
newCallback_Communicator_flushBatchRequests(T* instance, void (T::*excb)(const ::Ice::Exception&),
                                            void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Communicator_flushBatchRequests<T>(instance, excb, sentcb);
}

/**
 * Type-safe asynchronous callback wrapper class used for calls to
 * Ice::Communicator::begin_flushBatchRequests.
 * Create a wrapper instance by calling ::Ice::newCallback_Communicator_flushBatchRequests.
 * \headerfile Ice/Ice.h
 */
template<class T, typename CT>
class Callback_Communicator_flushBatchRequests : public Callback_Communicator_flushBatchRequests_Base,
                                                 public ::IceInternal::OnewayCallback<T, CT>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception& , const CT&);
    typedef void (T::*Sent)(bool , const CT&);

    Callback_Communicator_flushBatchRequests(const TPtr& obj, Exception excb, Sent sentcb)
        : ::IceInternal::OnewayCallback<T, CT>(obj, 0, excb, sentcb)
    {
    }

    /// \cond INTERNAL
    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        ::Ice::CommunicatorPtr communicator = result->getCommunicator();
        assert(communicator);
        try
        {
            communicator->end_flushBatchRequests(result);
            assert(false);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::Callback<T, CT>::exception(result, ex);
        }
    }
    /// \endcond
};

/**
 * Type-safe asynchronous callback wrapper class used for calls to
 * Ice::Communicator::begin_flushBatchRequests.
 * Create a wrapper instance by calling ::Ice::newCallback_Communicator_flushBatchRequests.
 */
template<class T, typename CT> Callback_Communicator_flushBatchRequestsPtr
newCallback_Communicator_flushBatchRequests(const IceUtil::Handle<T>& instance,
                                            void (T::*excb)(const ::Ice::Exception&, const CT&),
                                            void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Communicator_flushBatchRequests<T, CT>(instance, excb, sentcb);
}

/**
 * Type-safe asynchronous callback wrapper class used for calls to
 * Ice::Communicator::begin_flushBatchRequests.
 * Create a wrapper instance by calling ::Ice::newCallback_Communicator_flushBatchRequests.
 */
template<class T, typename CT> Callback_Communicator_flushBatchRequestsPtr
newCallback_Communicator_flushBatchRequests(T* instance, void (T::*excb)(const ::Ice::Exception&, const CT&),
                                            void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Communicator_flushBatchRequests<T, CT>(instance, excb, sentcb);
}

}
#endif

#endif
