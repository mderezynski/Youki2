#include <config.h>
#include <cstdlib>
#include <glib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#include "mpx/mpx-signals.hh"

namespace MPX
{
  static SignalHandler
  install_handler_full (int           signal_number,
                        SignalHandler handler,
                        int*          signals_to_block,
                        std::size_t   n_signals)
  {
      struct sigaction action, old_action;

      action.sa_handler = handler;
      action.sa_flags = SA_RESTART;

      sigemptyset (&action.sa_mask);

      for (std::size_t i = 0; i < n_signals; i++)
          sigaddset (&action.sa_mask, signals_to_block[i]);

      if (sigaction (signal_number, &action, &old_action) == -1)
      {
          g_warning (G_STRLOC ": Failed to install handler for signal %d", signal_number);
          return 0;
      }

      return old_action.sa_handler;
  }

  // A version of signal() that works more reliably across different
  // platforms. It:
  // a. restarts interrupted system calls
  // b. does not reset the handler
  // c. blocks the same signal within the handler
  //
  // (adapted from Unix Network Programming Vol. 1)

  SignalHandler
  install_handler (int           signal_number,
                   SignalHandler handler)
  {
      return install_handler_full (signal_number, handler, NULL, 0);
  }


} // MPX
