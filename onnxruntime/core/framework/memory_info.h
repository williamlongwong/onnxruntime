#pragma once

#include "core/graph/basic_types.h"
#include "core/framework/alloc_kind.h"
#include "core/framework/data_types.h"
#include "core/framework/execution_plan_base.h"
#include "core/framework/mem_pattern.h"
#include "core/framework/tensor.h"
#include "core/framework/ort_value_name_idx_map.h"

#include <iomanip>

namespace onnxruntime {
using IntervalT = std::pair<size_t, size_t>;
using OrtValueIndex = int;
using OrtValueName = std::string;
//TODO: need to extend this enum to include finner-grained decomposition
enum MLValueTensorType {
  WEIGHT = 0,
  FWD_ACTIVATION,
  GRADIENT,
  Unknown,
};

struct MemoryInfoPerTensor {
  MemoryInfoPerTensor() = default;
  MLValueTensorType tensor_type{Unknown};
  OrtValueIndex mlvalue_index{0};
  OrtValueName mlvalue_name{""};

  AllocPlanPerValue alloc_plan;

  MemoryBlock planned_block;
  MemoryBlock allocated_block;

  bool dynamic_allocation{false};
  friend std::ostream& operator<<(std::ostream& out, const MemoryInfoPerTensor& mem_info_per_tensor);
};

class MemoryInfo {
 public:
  MemoryInfo() : iteration_(0) {
    time_t now_c = std::time(0);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now_c);
    char buffer[80];
    asctime_s(buffer, &timeinfo);
    memory_info_file = "memory_info_file_" + std::string(buffer);
  }
  void GenerateMemoryMap(const SequentialExecutionPlan* execution_plan, const OrtValueNameIdxMap& value_name_idx_map);
  void RecordMemoryPatternInfo(const MemoryPatternGroup& mem_patterns);
  void RecordDeviceAllocInfo(const std::unordered_map<int, OrtValue>& tensor_map);
  void RecordActivationPatternInfo(const MemoryPatternGroup& mem_patterns);
  void SetDynamicAllocation(const OrtValueIndex idx);
  void RecordTensorDeviceAllocInfo(const OrtValueIndex idx, const OrtValue& value);
  void RecordInputMemoryInfo(const std::vector<int>& feed_mlvalue_idxs, const std::vector<OrtValue>& feeds);

  void PrintMemoryInfoForLocation(const logging::Logger& /*logger*/, const OrtDevice::DeviceType location);
  void MemoryInfo::WriteMemoryInfoToFile();
  void SetIteration(size_t iteration) {
    iteration_ = iteration;
  }

 private:
  std::unordered_map<OrtValueIndex, MemoryInfoPerTensor> tensor_memoryinfo_map_;
  //TODO: The dynamic and statically planned alignments may not be the same, need to check
  static const int alignment = 256;
  size_t iteration_ = 0;
  std::string memory_info_file;
};

}  // namespace onnxruntime