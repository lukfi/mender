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

#include <common/processes.hpp>

#include <string>
#include <string_view>

#include <common/events.hpp>
#include <common/io.hpp>

using namespace std;

namespace mender::common::processes {

namespace events = mender::common::events;
namespace io = mender::common::io;

//class ProcessReader : virtual public io::AsyncReader {
//public:
//	ProcessReader(events::EventLoop &loop);
//	~ProcessReader();

//	error::Error AsyncRead(vector<uint8_t>::iterator start, vector<uint8_t>::iterator end, function<void(error::Error &err)> handler) override;
//	void Cancel() override;
//};

Process::~Process() {
	if (stdout_pipe_ >= 0) {
		close(stdout_pipe_);
	}
	if (stderr_pipe_ >= 0) {
		close(stderr_pipe_);
	}
}

error::Error Process::Start(std::function<void(const char *bytes, size_t n)> read_stdout) {
	proc_.reset(new tpl::Process(this->args_, "", read_stdout));

	if (proc_->get_id() == -1) {
		return MakeError(ProcessesErrorCode::SpawnError, "Failed to spawn '" + this->args_[0] + "'");
	}

	return error::NoError;
}

int Process::Wait() {
	if (proc_) {
		exit_status_ = proc_->get_exit_status();
		proc_.reset();
	}
	return exit_status_;
}

ExpectedLineData Process::GenerateLineData() {
	if (this->args_.size() == 0) {
		return expected::unexpected(MakeError(
			ProcessesErrorCode::SpawnError, "No arguments given, cannot spawn a process"));
	}

	string trailing_line;
	vector<string> ret;
	proc_.reset(new tpl::Process(this->args_, "", [&trailing_line, &ret](const char *bytes, size_t len) {
			auto bytes_view = string_view(bytes, len);
			size_t line_start_idx = 0;
			size_t line_end_idx = bytes_view.find("\n", 0);
			if ((trailing_line != "") && (line_end_idx != string_view::npos)) {
				ret.push_back(trailing_line + string(bytes_view, 0, line_end_idx));
				line_start_idx = line_end_idx + 1;
				line_end_idx = bytes_view.find("\n", line_start_idx);
				trailing_line = "";
			}

			while ((line_start_idx < (len - 1)) && (line_end_idx != string_view::npos)) {
				ret.push_back(string(bytes_view, line_start_idx, (line_end_idx - line_start_idx)));
				line_start_idx = line_end_idx + 1;
				line_end_idx = bytes_view.find("\n", line_start_idx);
			}

			if ((line_end_idx == string_view::npos) && (line_start_idx != (len - 1))) {
				trailing_line += string(bytes_view, line_start_idx, (len - line_start_idx));
			}
		}));

	auto id = proc_->get_id();

	// waits for the process to finish
	// TODO: log exit status != 0? Or error?
	Wait();

	if (trailing_line != "") {
		ret.push_back(trailing_line);
	}

	if (id == -1) {
		return expected::unexpected(
			MakeError(ProcessesErrorCode::SpawnError, "Failed to spawn '" + this->args_[0] + "'"));
	}

	return ExpectedLineData(ret);
}

//ExpectedAsyncReaderPtr Process::GetStdoutReader() {
//	int fds[2];
//	int ret = pipe(fds);
//	if (ret < 0) {
//		int err = errno;
//		return expected::unexpected(error::MakeError(generic_category().default_error_condition(err), "Could not get process stdout reader"));
//	}

//	// TODO: No, not int, use asio.
//	stdout_pipe_ = fds[0];
//}

//ExpectedAsyncReaderPtr Process::GetStderrReader() {
//}

void Process::Terminate() {
	if (proc_) {
		proc_->kill(false);
	}
}

void Process::Kill() {
	if (proc_) {
		proc_->kill(true);
	}
}

} // namespace mender::common::processes
