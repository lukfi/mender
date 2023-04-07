// Copyright 2023 Northern.tech AS
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

#include <mender-update/update_module/v3/update_module_call.hpp>

#include <iostream>
#include <sstream>
#include <common/processes.hpp>

namespace mender {
namespace update {
namespace update_module {
namespace v3 {

namespace error = mender::common::error;
namespace procs = mender::common::processes;

ExpectedExitStatus UpdateModuleCall::CallState(State state, string& procOut)
{
	std::stringstream ss;
	procs::Process proc({"/home/lukfi/test.proc"}); // LF# todo
	auto processStart = proc.Start([&ss](const char *data, size_t size){ ss.write(data, size); });
	if (error::NoError != processStart) {
		return expected::unexpected(processStart);
	}

	int exitStatus = proc.Wait();
	procOut = ss.str();
	return exitStatus;
}

}
}
}
}
