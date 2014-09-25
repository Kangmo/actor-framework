/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2014                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#ifndef CAF_BLOCKING_UNTYPED_ACTOR_HPP
#define CAF_BLOCKING_UNTYPED_ACTOR_HPP

#include "caf/none.hpp"

#include "caf/on.hpp"
#include "caf/extend.hpp"
#include "caf/behavior.hpp"
#include "caf/local_actor.hpp"
#include "caf/typed_actor.hpp"
#include "caf/mailbox_element.hpp"
#include "caf/response_handle.hpp"

#include "caf/detail/type_traits.hpp"

#include "caf/mixin/sync_sender.hpp"
#include "caf/mixin/mailbox_based.hpp"

namespace caf {

/**
 * A thread-mapped or context-switching actor using a blocking
 * receive rather than a behavior-stack based message processing.
 * @extends local_actor
 */
class blocking_actor
    : public extend<local_actor, blocking_actor>::
             with<mixin::mailbox_based,
                  mixin::sync_sender<blocking_response_handle_tag>::impl> {
 public:
  class functor_based;

  blocking_actor();

  ~blocking_actor();

  /**************************************************************************
   *       utility stuff and receive() member function family       *
   **************************************************************************/
  using timeout_type = std::chrono::high_resolution_clock::time_point;

  class receive_loop_helper {
   public:
    using loop_fun = std::function<void (behavior&)>;
    receive_loop_helper(loop_fun loop);

    template <class... Ts>
    void operator()(Ts&&... args);

  private:
    loop_fun m_loop;
  };

  class do_receive_helper {
   public:
    do_receive_helper(blocking_actor* self, behavior bhvr);

    template <class Statement>
    void until(Statement stmt);

    inline void until(const bool& bvalue) {
      until([&] { return bvalue; });
    }

   private:
    blocking_actor* m_self;
    behavior m_bhvr;
  };

  /**
   * Dequeues the next message from the mailbox that is
   * matched by given behavior.
   */
  template <class... Ts>
  void receive(Ts&&... args) {
    static_assert(sizeof...(Ts), "at least one argument required");
    behavior bhvr{std::forward<Ts>(args)...};
    dequeue(bhvr);
  }

  /**
   * Semantically equal to: `for (;;) { receive(...); }`, but does
   * not cause a temporary behavior object per iteration.
   */
  template <class... Ts>
  void receive_loop(Ts&&... args) {
    behavior bhvr{std::forward<Ts>(args)...};
    for (;;) dequeue(bhvr);
  }

  /**
   * Receives messages for range `[begin, first)`.
   * Semantically equal to:
   * `for ( ; begin != end; ++begin) { receive(...); }`.
   *
   * **Usage example:**
   * ~~~
   * int i = 0;
   * receive_for(i, 10) (
   *   on(atom("get")) >> [&]() -> message {
   *     return {"result", i};
   *   }
   * );
   * ~~~
   */
  template <class BeginIter, class EndIter>
  receive_loop_helper receive_for(BeginIter& begin, EndIter end) {
    auto loop = [this, &begin, end](behavior& bhvr) {
      for (; begin != end; ++begin) {
        dequeue(bhvr);
      }
    };
    return receive_loop_helper(loop);
  }

  /**
   * Receives messages as long as `stmt` returns true.
   * Semantically equal to: `while (stmt()) { receive(...); }`.
   *
   * **Usage example:**
   * ~~~
   * int i = 0;
   * receive_while([&]() { return (++i <= 10); })
   * (
   *   on<int>() >> int_fun,
   *   on<float>() >> float_fun
   * );
   * ~~~
   */
  template <class Stmt>
  receive_loop_helper receive_while(Stmt stmt) {
    static_assert(std::is_same<bool, decltype(stmt())>::value,
                 "stmt() does not return bool");
    auto loop = [=](behavior& bhvr) {
      while (stmt()) {
        dequeue(bhvr);
      }
    };
    return receive_loop_helper(loop);
  }

  /**
   * Receives messages until `stmt` returns true.
   *
   * Semantically equal to:
   * `do { receive(...); } while (stmt() == false);`
   *
   * **Usage example:**
   * ~~~
   * int i = 0;
   * do_receive
   * (
   *   on<int>() >> int_fun,
   *   on<float>() >> float_fun
   * )
   * .until([&]() { return (++i >= 10); };
   * ~~~
   */
  template <class... Ts>
  do_receive_helper do_receive(Ts&&... args) {
    return do_receive_helper(this, behavior{std::forward<Ts>(args)...});
  }

  optional<behavior&> sync_handler(message_id msg_id) {
    auto i = m_sync_handler.find(msg_id);
    if (i != m_sync_handler.end()) return i->second;
    return none;
  }

  /**
   * Blocks this actor until all other actors are done.
   */
  void await_all_other_actors_done();

  /**
   * Implements the actor's behavior.
   */
  virtual void act() = 0;

  /** @cond PRIVATE */

  // required from invoke_policy; unused in blocking actors
  inline void remove_handler(message_id) {}

  // required by receive() member function family
  inline void dequeue(behavior&& bhvr) {
    behavior tmp{std::move(bhvr)};
    dequeue(tmp);
  }

  // required by receive() member function family
  inline void dequeue(behavior& bhvr) {
    dequeue_response(bhvr, invalid_message_id);
  }

  // implemented by detail::proper_actor
  virtual void dequeue_response(behavior& bhvr, message_id mid) = 0;

  /** @endcond */

 private:
  std::map<message_id, behavior> m_sync_handler;
};

template <class... Ts>
void blocking_actor::receive_loop_helper::operator()(Ts&&... args) {
  behavior bhvr{std::forward<Ts>(args)...};
  m_loop(bhvr);
}

template <class Statement>
void blocking_actor::do_receive_helper::until(Statement stmt) {
  do {
    m_self->dequeue(m_bhvr);
  } while (stmt() == false);
}

class blocking_actor::functor_based : public blocking_actor {
 public:
  using act_fun = std::function<void(blocking_actor*)>;

  template <class F, class... Ts>
  functor_based(F f, Ts&&... vs) {
    using trait = typename detail::get_callable_trait<F>::type;
    using arg0 = typename detail::tl_head<typename trait::arg_types>::type;
    blocking_actor* dummy = nullptr;
    constexpr bool uses_selfptr = std::is_same<arg0, blocking_actor*>::value;
    std::integral_constant<bool, uses_selfptr> token;
    create(dummy, token, f, std::forward<Ts>(vs)...);
  }

 protected:
  void act() override;

 private:
  void create(blocking_actor*, act_fun);

  template <class Actor, typename F, class... Ts>
  void create(Actor* dummy, std::true_type, F f, Ts&&... vs) {
    create(dummy, std::bind(f, std::placeholders::_1, std::forward<Ts>(vs)...));
  }

  template <class Actor, typename F, class... Ts>
  void create(Actor* dummy, std::false_type, F f, Ts&&... vs) {
    std::function<void()> fun = std::bind(f, std::forward<Ts>(vs)...);
    create(dummy, [fun](Actor*) { fun(); });
  }

  act_fun m_act;
};

} // namespace caf

#endif // CAF_BLOCKING_UNTYPED_ACTOR_HPP
