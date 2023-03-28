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

#include <artifact/v3/payload/payload.hpp>

#include <string>
#include <fstream>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <artifact/tar/tar.hpp>

#include <common/processes.hpp>
#include <common/testing.hpp>

using namespace std;

namespace io = mender::common::io;
namespace tar = mender::tar;
namespace processes = mender::common::processes;
namespace mendertesting = mender::common::testing;
namespace payload = mender::artifact::v3::payload;

class PayloadTestEnv : public testing::Test {
public:
protected:
	static void SetUpTestSuite() {
		string script = R"(#! /bin/sh

    DIRNAME=$(dirname $0)

		# Create small tar payload file
		echo foobar > ${DIRNAME}/testdata
		tar cvf ${DIRNAME}/test.tar ${DIRNAME}/testdata

		exit 0
		)";

		const string script_fname = tmpdir->Path() + "/test-script.sh";

		std::ofstream os(script_fname.c_str(), std::ios::out);
		os << script;
		os.close();

		int ret = chmod(script_fname.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
		ASSERT_EQ(ret, 0);


		processes::Process proc({script_fname});
		auto ex_line_data = proc.GenerateLineData();
		ASSERT_TRUE(ex_line_data);
		EXPECT_EQ(proc.GetExitStatus(), 0) << "error message: " + ex_line_data.error().message;
	}

	static void TearDownTestSuite() {
		tmpdir.reset();
	}

	static unique_ptr<mendertesting::TemporaryDirectory> tmpdir;
};

unique_ptr<mendertesting::TemporaryDirectory> PayloadTestEnv::tmpdir =
	unique_ptr<mendertesting::TemporaryDirectory>(new mendertesting::TemporaryDirectory());

TEST_F(PayloadTestEnv, TestPayloadSuccess) {
	std::fstream fs {tmpdir->Path() + "/test.tar"};

	mender::common::io::StreamReader sr {fs};

	mender::tar::Reader tar_reader {sr};

	mender::tar::Entry tar_entry = tar_reader.Next().value();

	ASSERT_THAT(tar_entry.Name(), testing::EndsWith("testdata"));

	payload::Reader p = payload::Verify(
		tar_entry, "aec070645fe53ee3b3763059376134f058cc337247c978add178b6ccdfb0019f");

	auto discard_writer = io::Discard {};

	auto err = io::Copy(discard_writer, p);

	EXPECT_FALSE(err) << "Got unexpected error: " << err.message;
}

TEST_F(PayloadTestEnv, TestPayloadFailure) {
	std::fstream fs {tmpdir->Path() + "/test.tar"};

	mender::common::io::StreamReader sr {fs};

	mender::tar::Reader tar_reader {sr};

	mender::tar::Entry tar_entry = tar_reader.Next().value();

	ASSERT_THAT(tar_entry.Name(), testing::EndsWith("testdata"));

	payload::Reader p = payload::Verify(
		tar_entry,
		// Ends with (e) not (f)
		"aec070645fe53ee3b3763059376134f058cc337247c978add178b6ccdfb0019e");

	auto discard_writer = io::Discard {};

	auto err = io::Copy(discard_writer, p);

	EXPECT_TRUE(err);
}
