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

#include <mender-update/update_module/v3/update_module.hpp>
#include <mender-update/update_module/v3/update_module_call.hpp>

#include <common/error.hpp>
#include <common/expected.hpp>

#include <common/processes.hpp>

#include <iostream>

namespace mender {
namespace update {
namespace update_module {
namespace v3 {

namespace error = mender::common::error;
namespace expected = mender::common::expected;

namespace procs = mender::common::processes;

static error::Error CallStateNoOutput(State state) {
	UpdateModuleCall call;
	std::string processStdOut;
	auto procExit = call.CallState(state, processStdOut);
	if (!procExit) {
		return procExit.error();
	} else if (procExit.value() < 0) {
		return error::MakeError(error::GenericError, "Process exited with error");
	}
	return error::NoError;
}

error::Error UpdateModule::Download() {
	return error::NoError;
}

error::Error UpdateModule::ArtifactInstall() {
	return CallStateNoOutput(State::ArtifactInstall);
}

ExpectedRebootAction UpdateModule::NeedsReboot() {
	UpdateModuleCall call;
	std::string processStdOut;
	auto procExit = call.CallState(State::ArtifactInstall, processStdOut);
	if (!procExit) {
		return expected::unexpected(procExit.error());
	} else if (procExit.value() < 0) {
		 // LF# return error
	}
	if (0 == processStdOut.find("Yes")) {
		return ExpectedRebootAction(RebootAction::Yes);
	} else if (0 == processStdOut.find("No")) {
		return ExpectedRebootAction(RebootAction::No);
	} else if (0 == processStdOut.find("Automatic")) {
		return ExpectedRebootAction(RebootAction::Automatic);
	}
	return expected::unexpected(error::MakeError(error::GenericError, "Unexpected output from the process")); // LF# correct the error
}

error::Error UpdateModule::ArtifactReboot() {
	return CallStateNoOutput(State::ArtifactReboot);
}

error::Error UpdateModule::ArtifactCommit() {
	return CallStateNoOutput(State::ArtifactCommit);
}

expected::ExpectedBool UpdateModule::SupportsRollback() {
	UpdateModuleCall call;
	std::string processStdOut;
	auto procExit = call.CallState(State::ArtifactInstall, processStdOut);
	if (!procExit) {
		return expected::unexpected(procExit.error());
	} else if (procExit.value() < 0) {
		 // LF# return error
	}
	if (0 == processStdOut.find("Yes")) {
		return expected::ExpectedBool(true);
	} else if (0 == processStdOut.find("No")) {
		return expected::ExpectedBool(false);
	}
	return expected::unexpected(error::MakeError(error::GenericError, "Unexpected output from the process")); // LF# correct the error
}

error::Error UpdateModule::ArtifactRollback() {
	return CallStateNoOutput(State::ArtifactRollback);
}

error::Error UpdateModule::ArtifactVerifyReboot() {
	return CallStateNoOutput(State::ArtifactVerifyReboot);
}

error::Error UpdateModule::ArtifactRollbackReboot() {
	return CallStateNoOutput(State::ArtifactRollbackReboot);
}

error::Error UpdateModule::ArtifactVerifyRollbackReboot() {
	return CallStateNoOutput(State::ArtifactVerifyRollbackReboot);
}

error::Error UpdateModule::ArtifactFailure() {
	return CallStateNoOutput(State::ArtifactFailure);
}

error::Error UpdateModule::Cleanup() {
	return CallStateNoOutput(State::Cleanup);
}

} // namespace v3
} // namespace update_module
} // namespace update
} // namespace mender
