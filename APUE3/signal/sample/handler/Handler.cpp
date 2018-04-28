#include "Handler.h"
#include <signal.h>
#include <stdlib.h>
#include <iostream>
#include "../utility/utility.h"

Handler::Handler() {
		
}

Handler::~Handler() {

}

void Handler::OnSignal(int id) {
	switch(id) {
	case SIGINT: {
		utility::exit_with_msg("SIGINT get, Exiting now...", EXIT_FAILURE);
		break;
	}
	default:
		break;		
	}
}
