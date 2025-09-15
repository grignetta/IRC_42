#ifndef SIGNALS_HPP
# define SIGNALS_HPP

# include <csignal>
# include <cstdlib> 

extern volatile sig_atomic_t g_signal;

void handleSignal(int signum);
void setupSignalHandlers();

#endif
