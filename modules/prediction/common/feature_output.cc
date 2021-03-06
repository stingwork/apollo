/******************************************************************************
 * Copyright 2017 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include "modules/prediction/common/feature_output.h"

#include <vector>

#include "cyber/common/file.h"
#include "modules/common/util/string_util.h"
#include "modules/prediction/common/prediction_system_gflags.h"

namespace apollo {
namespace prediction {

using apollo::common::util::StrCat;

Features FeatureOutput::features_;
ListDataForLearning FeatureOutput::list_data_for_learning_;
ListPredictionResult FeatureOutput::list_prediction_result_;
ListFrameEnv FeatureOutput::list_frame_env_;
std::size_t FeatureOutput::idx_feature_ = 0;
std::size_t FeatureOutput::idx_learning_ = 0;
std::size_t FeatureOutput::idx_prediction_result_ = 0;
std::size_t FeatureOutput::idx_frame_env_ = 0;

void FeatureOutput::Close() {
  ADEBUG << "Close feature output";
  switch (FLAGS_prediction_offline_mode) {
    case 1: {
      WriteFeatureProto();
      break;
    }
    case 2: {
      WriteDataForLearning();
      break;
    }
    case 3: {
      WritePredictionResult();
      break;
    }
    case 4: {
      WriteFrameEnv();
      break;
    }
    default: {
      // No data dump
      break;
    }
  }
  Clear();
}

void FeatureOutput::Clear() {
  idx_feature_ = 0;
  idx_learning_ = 0;
  features_.Clear();
  list_data_for_learning_.Clear();
}

bool FeatureOutput::Ready() {
  Clear();
  return true;
}

void FeatureOutput::InsertFeatureProto(const Feature& feature) {
  features_.add_feature()->CopyFrom(feature);
}

void FeatureOutput::InsertDataForLearning(
    const Feature& feature, const std::vector<double>& feature_values,
    const std::string& category) {
  DataForLearning* data_for_learning =
      list_data_for_learning_.add_data_for_learning();
  data_for_learning->set_id(feature.id());
  data_for_learning->set_timestamp(feature.timestamp());
  for (size_t i = 0; i < feature_values.size(); ++i) {
    data_for_learning->add_features_for_learning(feature_values[i]);
  }
  data_for_learning->set_category(category);
  ADEBUG << "Insert [" << category
         << "] data for learning with size = " << feature_values.size();
}

void FeatureOutput::InsertPredictionResult(
    const int obstacle_id, const PredictionObstacle& prediction_obstacle) {
  PredictionResult* prediction_result =
      list_prediction_result_.add_prediction_result();
  prediction_result->set_id(obstacle_id);
  prediction_result->set_timestamp(prediction_obstacle.timestamp());
  for (int i = 0; i < prediction_obstacle.trajectory_size(); ++i) {
    prediction_result->add_trajectory()->CopyFrom(
        prediction_obstacle.trajectory(i));
  }
}

void FeatureOutput::InsertFrameEnv(const FrameEnv& frame_env) {
  list_frame_env_.add_frame_env()->CopyFrom(frame_env);
}

void FeatureOutput::WriteFeatureProto() {
  if (features_.feature_size() <= 0) {
    ADEBUG << "Skip writing empty feature.";
  } else {
    const std::string file_name = StrCat(FLAGS_prediction_data_dir, "/feature.",
                                         std::to_string(idx_feature_), ".bin");
    cyber::common::SetProtoToBinaryFile(features_, file_name);
    features_.Clear();
    ++idx_feature_;
  }
}

void FeatureOutput::WriteDataForLearning() {
  if (list_data_for_learning_.data_for_learning().empty()) {
    ADEBUG << "Skip writing empty data_for_learning.";
  } else {
    const std::string file_name =
        StrCat(FLAGS_prediction_data_dir, "/datalearn.",
               std::to_string(idx_learning_), ".bin");
    cyber::common::SetProtoToBinaryFile(list_data_for_learning_, file_name);
    list_data_for_learning_.Clear();
    ++idx_learning_;
  }
}

void FeatureOutput::WritePredictionResult() {
  if (list_prediction_result_.prediction_result().empty()) {
    ADEBUG << "Skip writing empty prediction_result.";
  } else {
    const std::string file_name =
        StrCat(FLAGS_prediction_data_dir, "/prediction_result.",
               std::to_string(idx_prediction_result_), ".bin");
    cyber::common::SetProtoToBinaryFile(list_prediction_result_, file_name);
    list_prediction_result_.Clear();
    ++idx_prediction_result_;
  }
}

void FeatureOutput::WriteFrameEnv() {
  if (list_frame_env_.frame_env().empty()) {
    ADEBUG << "Skip writing empty prediction_result.";
  } else {
    const std::string file_name =
        StrCat(FLAGS_prediction_data_dir, "/frame_env.",
               std::to_string(idx_frame_env_), ".bin");
    cyber::common::SetProtoToBinaryFile(list_frame_env_, file_name);
    list_frame_env_.Clear();
    ++idx_frame_env_;
  }
}

int FeatureOutput::Size() { return features_.feature_size(); }

int FeatureOutput::SizeOfDataForLearning() {
  return list_data_for_learning_.data_for_learning_size();
}

int FeatureOutput::SizeOfPredictionResult() {
  return list_prediction_result_.prediction_result_size();
}

int FeatureOutput::SizeOfFrameEnv() { return list_frame_env_.frame_env_size(); }

}  // namespace prediction
}  // namespace apollo
