#include <lib/pcmWrapper.h>
#include <papi.h>
#include <iostream>

int PcmWrapper::_papi_evt_set = PAPI_NULL;
long long PcmWrapper::_papi_before;
long long PcmWrapper::_papi_after;

void PcmWrapper::init()
{
	int retval = PAPI_library_init(PAPI_VER_CURRENT);

	if (retval != PAPI_VER_CURRENT)
	{
		std::cerr << "PAPI_library_init() failed\n";
		return;
	}

	if (PAPI_create_eventset(&_papi_evt_set) != PAPI_OK)
	{
		std::cerr << "PAPI_create_eventset() failed\n";
		return;
	}

	if (PAPI_add_event(_papi_evt_set, PAPI_TOT_INS) != PAPI_OK)
	{
		std::cerr << "PAPI_add_event() failed\n";
		return;
	}
}

void PcmWrapper::startRecord()
{
	PAPI_start(_papi_evt_set);
	PAPI_read(_papi_evt_set, &_papi_before);
}

void PcmWrapper::stopRecord()
{
	PAPI_stop(_papi_evt_set, &_papi_after);
}

long long PcmWrapper::getInstructionsRetired()
{
	return _papi_after - _papi_before;
}
