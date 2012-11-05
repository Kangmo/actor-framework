Version 0.5.2
-------------

__2012-11-05__

- Fixed Bug in CMake when compiling w/o Boost.Context
- Added `--bulid-static` and `--build-static-only` flags to configure script
- Moved `benchmarks` folder to own repository

Version 0.5.1
-------------

__2012-11-03__

- Added `make_response_handle` which allows an actor to reply to message later
- Replace `continuable_writer` with `continuable_io : continuable_reader`
- No more multiple inheritance in `default_peer` (derives `continuable_io`)
- MM reports IO failures to corresponding objects
- New behavior in default protocol regarding outgoing messages:
  * `default_peer` uses FIFO queue for outgoing messages
  * queue stores messages even if no active connection is available
  * new connections then check for previously enqueued messages first

Version 0.5
-----------

__2012-10-26__

- New logging facility
  * must be enabled using --with-cppa-log-level=LEVEL (TRACE-ERROR)
  * --enable-debug also enables ERROR log level implicitly
  * output files are named libcppa_PID_TIMESTAMP.log
  * log format is Log4j-like
- New middleman (MM) architecture
  * MM multiplexes sockets but no longer knows communication internas
  * new protocol interface encapsulates any communication
  * users can add new communication protocols to MM
  * previously used binary protocol is not called 'DEFAULT'
  * MM provides run_later function to hook code into MM event-loop
- New class: `weak_intrusive_ptr`
  * `ref_counted` has protected destructor to enforce use of `request_deletion`
  * default `request_deletion` calls `delete`
  * `enable_weak_ptr_mixin` overrides `request_deletion` to invalidate weak ptrs
- Fixed issue #75: peers hold weak pointers to proxies (breaks cyclic refs)
- `actor_companion_mixin` allows non-actor classes to communicate as/to actors
- `actor_proxy` became an abstract class; must be implemented for each protocol
- Removed global proxy cache singleton
- `actor_addressing` manages proxies; must be implemented for each protocol
- New factory function: `make_counted` (similar to std::make_shared)
- Bugfix: `reply` matches correct message on nested receives

Version 0.4.2
-------------

__2012-10-1__

- Refactored announce
  * accept recursive containers, e.g.,  `vector<vector<double>>`
  * allow user-defined types as members of announced types
  * all-new, policy-based implementation
- Use `poll` rather than `select` in middleman (based on the patch by ArtemGr)

Version 0.4.1
-------------

__2012-08-22__

- Bugfix: `shutdown` caused segfault if no scheduler or middleman was started

Version 0.4
-----------

__2012-08-20__

- New network layer implementation
- Added acceptor and input/output stream interfaces
- Added overload for `publish` and `remote_actor` using the new interfaces
- Changed group::add_module to take unique_ptr rather than a raw pointer
- Refactored serialization process for group_ptr
- Changed anyonymous groups to use the implementation of the "local" module
- Added scheduled_and_hidden policy for system-internal, event-based actors
- Enabled serialization of floating point values
- Added `shutdown` function
- Implemented broker-based forwarding of local groups for 'pseudo multicast'
- Added `then` and `await` member functions to message_future
- Do not send more than one response message with `reply`

Version 0.3.3
-------------

__2012-08-09__

- Bugfix: serialize message id for synchronous messaging
- Added macro to unit_testing/CMakeLists.txt for less verbose CMake setup
- Added function "forward_to" to enable transparent forwarding of sync requests
- Removed obsolete files (gen_server/* and queue_performances/*)
- Bugfix: avoid possible stack overflow in debug mode for test__spawn
- Added functions "send_tuple", "sync_send_tuple" and "reply_tuple"
- Let "make" fail on first error in dual-build mode
- Added rvalue overload for receive_loop
- Added "delayed_send_tuple" and "delayed_reply_tuple"

Version 0.3.2
-------------

__2012-07-30__

- Bugfix: added 'bool' to the list of announced types

Version 0.3.1
-------------

__2012-07-27__

- Bugfix: always return from a synchronous handler if a timeout occurs
- Bugfix: request next timeout after timeout handler invocation if needed

Version 0.3
-----------

__2012-07-25__

- Implemented synchronous messages
- The function become() no longer accepts pointers
- Provide --disable-context-switching option to bypass Boost.Context if needed
- Configure script to hide CMake details behind a nice interface
- Include "tuple_cast.hpp" in "cppa.hpp"
- Added forwarding header "cppa_fwd.hpp"
- Allow raw read & write operations in synchronization interface
- Group subscriptions are no longer attachables

Version 0.2.1
-------------

__2012-07-02__

- More efficient behavior implementation
- Relaxed definition of `become` to accept const lvalue references as well

Version 0.2
-----------

__2012-06-29__

- Removed `become_void` [use `quit` instead]
- Renamed `future_send` to `delayed_send`
- Removed `stacked_actor`; moved functionality to `event_based_actor`
- Renamed `fsm_actor` to `sb_actor`
- Refactored `spawn`: `spawn(new T(...))` => `spawn<T>(...)`
- Implemented `become` & `unbecome` for context-switching & thread-mapped actors
- Moved `become` & `unbecome` to local_actor
- Ported libcppa from `<ucontext.h>` to `Boost.Context` library