#pragma once

namespace zpp_lib {

// we expect CONFIG_NUM_PREEMPT_PRIORITIES to be at least 
#if CONFIG_NUM_PREEMPT_PRIORITIES < 8
#error zpp_lib requires CONFIG_NUM_PREEMPT_PRIORITIES >= 8
#endif

// Preemptable thread priority values
enum class PreemptableThreadPriority {
  PriorityIdle          =  CONFIG_NUM_PREEMPT_PRIORITIES - 1,  ///< Reserved for Idle thread.
  PriorityLow           =  CONFIG_NUM_PREEMPT_PRIORITIES - 2,  ///< Priority: low
  PriorityBelowNormal   =  CONFIG_NUM_PREEMPT_PRIORITIES - 3,  ///< Priority: below normal
  PriorityNormal        =  CONFIG_NUM_PREEMPT_PRIORITIES - 4,  ///< Priority: normal
  PriorityAboveNormal   =  CONFIG_NUM_PREEMPT_PRIORITIES - 5,  ///< Priority: above normal
  PriorityHigh          =  CONFIG_NUM_PREEMPT_PRIORITIES - 6,  ///< Priority: high
  PriorityRealtime      =  CONFIG_NUM_PREEMPT_PRIORITIES - 7,  ///< Priority: realtime
  PriorityISR           =  -CONFIG_NUM_PREEMPT_PRIORITIES      ///< Highest priority (Reserved for ISR deferred thread)
};
 
} // namespace zpp_lib