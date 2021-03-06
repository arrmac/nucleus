/*
 * Copyright 2018 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "nucleus/io/fastq_reader.h"

#include "nucleus/testing/protocol-buffer-matchers.h"
#include "nucleus/testing/test_utils.h"
#include "nucleus/util/utils.h"

#include <gmock/gmock-generated-matchers.h>
#include <gmock/gmock-matchers.h>
#include <gmock/gmock-more-matchers.h>

#include "tensorflow/core/platform/test.h"

namespace nucleus {

using std::vector;

using ::testing::Pointwise;

constexpr char kFastqFilename[] = "test_reads.fastq";
constexpr char kGzippedFastqFilename[] = "test_reads.fastq.gz";

void CreateRecord(const string& id, const string& description,
                  const string& sequence, const string& quality,
                  nucleus::genomics::v1::FastqRecord* record) {
  record->set_id(id);
  record->set_description(description);
  record->set_sequence(sequence);
  record->set_quality(quality);
}

class FastqReaderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    nucleus::genomics::v1::FastqRecord first, second, third;
    CreateRecord("NODESC:header", "", "GATTACA", "BB>B@FA", &first);
    CreateRecord(
        "M01321:49:000000000-A6HWP:1:1101:17009:2216", "1:N:0:1",
        "CGTTAGCGCAGGGGGCATCTTCACACTGGTGACAGGTAACCGCCGTAGTAAAGGTTCCGCCTTTCACT",
        "AAAAABF@BBBDGGGG?FFGFGHBFBFBFABBBHGGGFHHCEFGGGGG?FGFFHEDG3EFGGGHEGHG",
        &second);
    CreateRecord("FASTQ", "contains multiple spaces in description",
                 "CGGCTGGTCAGGCTGACATCGCCGCCGGCCTGCAGCGAGCCGCTGC",
                 "FAFAF;F/9;.:/;999B/9A.DFFF;-->.AAB/FC;9-@-=;=.", &third);
    golden_ = {first, second, third};
  }

  vector<nucleus::genomics::v1::FastqRecord> golden_;
};

TEST_F(FastqReaderTest, NormalIterationWorks) {
  std::unique_ptr<FastqReader> reader = std::move(
      FastqReader::FromFile(GetTestData(kFastqFilename),
                            nucleus::genomics::v1::FastqReaderOptions())
          .ValueOrDie());

  EXPECT_THAT(as_vector(reader->Iterate()), Pointwise(EqualsProto(), golden_));
}

TEST_F(FastqReaderTest, GzippedIterationWorks) {
  auto opts = nucleus::genomics::v1::FastqReaderOptions();
  opts.set_compression_type(nucleus::genomics::v1::FastqReaderOptions::GZIP);
  std::unique_ptr<FastqReader> reader =
      std::move(FastqReader::FromFile(GetTestData(kGzippedFastqFilename), opts)
                    .ValueOrDie());

  EXPECT_THAT(as_vector(reader->Iterate()), Pointwise(EqualsProto(), golden_));
}
}  // namespace nucleus
