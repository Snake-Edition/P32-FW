#pragma once
#include "metric.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/// Set up the metrics handlers based on config store
void metrics_reconfigure();

bool are_metrics_enabled();

extern void metric_handler(metric_point_t *point);

#ifdef __cplusplus
}
#endif //__cplusplus
