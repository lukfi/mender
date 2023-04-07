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

#ifndef MENDER_COMMON_PROCESSES_HPP
#define MENDER_COMMON_PROCESSES_HPP

#include <string>
#include <vector>
#include <common/error.hpp>
#include <common/expected.hpp>

#include <config.h>

#ifdef MENDER_USE_TINY_PROC_LIB
#include <process.hpp>
#endif

namespace mender {
namespace common {
namespace processes {

#ifdef MENDER_USE_TINY_PROC_LIB
namespace tpl = TinyProcessLib;
#endif

using namespace std;

namespace error = mender::common::error;
namespace expected = mender::common::expected;

enum ProcessesErrorCode {
	NoError = 0,
	SpawnError,
};

class ProcessesErrorCategoryClass : public std::error_category {
public:
	const char *name() const noexcept override;
	string message(int code) const override;
};
extern const ProcessesErrorCategoryClass ProcessesErrorCategory;

error::Error MakeError(ProcessesErrorCode code, const string &msg);

using LineData = vector<string>;
using ExpectedLineData = expected::expected<LineData, error::Error>;

class Process {
public:
	Process(vector<string> args) :
		args_(args) {};
	~Process();

	error::Error Start(std::function<void (const char *, size_t)> read_stdout = nullptr);

	int Wait();

	int GetExitStatus() {
		return Wait();
	};
	ExpectedLineData GenerateLineData();

//	ExpectedAsyncReaderPtr GetStdoutReader();
//	ExpectedAsyncReaderPtr GetStderrReader();

	void Terminate();
	void Kill();

private:
#ifdef MENDER_USE_TINY_PROC_LIB
	unique_ptr<tpl::Process> proc_;

	int stdout_pipe_ {-1};
	int stderr_pipe_ {-1};
#endif
	vector<string> args_;
	int exit_status_ = -1;
};

} // namespace processes
} // namespace common
} // namespace mender

#endif // MENDER_COMMON_PROCESSES_HPP
