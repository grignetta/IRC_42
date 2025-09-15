#include "Signals.hpp"

volatile sig_atomic_t g_signal = 0;

void handleSignal(int signum)
{
	(void) signum;
	g_signal = 1; 
}

void setupSignalHandlers()
{
	std::signal(SIGINT, handleSignal);
	std::signal(SIGTERM, handleSignal);
	std::signal(SIGQUIT, handleSignal);
}