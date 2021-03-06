/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/example/Scope.h
 * \brief ???
 */

#ifndef __components_atomic_example_Scope_h__
#define __components_atomic_example_Scope_h__

namespace PAMI
{
  namespace Atomic
  {
    namespace Interface
    {
      ///
      /// \brief Node scope interface class
      ///
      template <class T>
      class NodeScope
      {
        public:
          inline NodeScope  () {};
          inline ~NodeScope () {};

          /// \note All classes that implement the NodeScope interface must
          ///       contain the \c _scoped boolean data memeber that is
          ///       accessed in this method interface.
          inline void setNodeScope ();
      };

      ///
      /// \brief Process scope interface class
      ///
      template <class T>
      class ProcessScope
      {
        public:
          inline ProcessScope  () {};
          inline ~ProcessScope () {};

          /// \note All classes that implement the NodeScope interface must
          ///       contain the \c _scoped boolean data memeber that is
          ///       accessed in this method interface.
          inline void setProcessScope ();
      };
    };
  };
};

template <class T>
inline bool PAMI::Atomic::Interface::NodeScope<T>::setNodeScope()
{
  static_cast<T*>(this)->setNodeScope_impl();
  return T::_scoped;
}

template <class T>
inline bool PAMI::Atomic::Interface::ProcessScope<T>::setProcessScope()
{
  static_cast<T*>(this)->setProcessScope_impl();
  return T::_scoped;
}

#endif
