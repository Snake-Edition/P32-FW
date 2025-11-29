#pragma once

#include <span>
#include <array>
#include <cassert>
#include <limits>
#include <cmath>

#include <utils/uncopyable.hpp>
#include <utils/progress.hpp>

template <class State>
struct ProgressMapperWorkflowStep {
    using Scale = uint8_t;

    /// External key (presumably FSM state) associated with the step
    State state;

    /// 'Size' of the step, relative to other steps.
    /// Determines how much progress span is associated with it
    Scale scale = 1;
};

template <class State>
class ProgressMapperWorkflow : Uncopyable {

public:
    using StepIndex = uint8_t;
    using Step = ProgressMapperWorkflowStep<State>;
    using StepScale = Step::Scale;

    struct StepData {
        /// Same as ProgressMapperWorkflowStep::state
        State state;

        /// Similar to ProgressMapperWorkflowStep::scale, but with scale of all previous steps accumulated
        uint8_t cumulative_scale;
    };

public:
    consteval ProgressMapperWorkflow() = default;

    constexpr const auto &steps() const {
        return steps_;
    }

    constexpr inline float scale_sum() const {
        return steps_[steps_.size() - 1].cumulative_scale;
    }

    constexpr static inline bool is_workflow_valid(StepIndex step_count) {
        return (step_count > 0) && (step_count < std::numeric_limits<StepIndex>::max());
    }

protected:
    consteval void setup(const std::span<StepData> &steps, const std::span<const Step> &params) {
        assert(is_workflow_valid(params.size()));
        assert(params.size() == steps.size());

        steps_ = steps;
        StepScale scale_accum = 0;

        auto step = steps.begin();
        for (const auto &param : params) {
            assert(param.scale > 0);

            scale_accum += param.scale;

            *step = StepData {
                .state = param.state,
                .cumulative_scale = scale_accum,
            };
            step++;
        }
    }

private:
    std::span<const StepData> steps_;
};

template <class State, std::size_t N>
class ProgressMapperWorkflowArray : public ProgressMapperWorkflow<State> {
public:
    using WorkflowBase = ProgressMapperWorkflow<State>;

public:
    consteval ProgressMapperWorkflowArray(const std::array<ProgressMapperWorkflowStep<State>, N> &params) {
        // max is reserved as for the initial value
        static_assert(WorkflowBase::is_workflow_valid(N));

        this->setup(data_, params);
    }

private:
    std::array<typename WorkflowBase::StepData, N> data_;
};

class BaseProgressMapper : Uncopyable {

public:
    using StepIndex = uint8_t;
    static constexpr StepIndex invalid_step = std::numeric_limits<StepIndex>::max();

public:
    inline ProgressPercent current_progress() const {
        return current_progress_;
    }

protected:
    BaseProgressMapper() = default;

protected:
    ProgressPercent current_progress_ = 0;

    /// Start progress of the origin step
    ProgressSpan current_step_span_;

    /// Index of the current step of the mapper
    StepIndex current_step_index_ = invalid_step;
};

template <class State>
class ProgressMapper : public BaseProgressMapper {

public:
    using Workflow = ProgressMapperWorkflow<State>;
    using StepIndex = Workflow::StepIndex;

public:
    ProgressMapper() = default;
    ProgressMapper(const Workflow &workflow) {
        setup(workflow);
    }

    /// Reset the progress mapper and set it up for the specified workflow
    void setup(const Workflow &workflow) {
        workflow_ = &workflow;
        current_progress_ = 0;
        current_step_span_ = ProgressSpan { 0, 0 };
        current_step_index_ = invalid_step;
    }

    /// Informs the progress mapper that the process in currently in \p state.
    /// \param normalized_progress determines progress within the current step
    /// \returns overall progress within the workflow
    ProgressPercent update_progress(const State state, const float normalized_progress) {
        current_progress_ = update_progress_span(state).map(normalized_progress);
        return current_progress_;
    }

    /// Informs the progress mapper that the process in currently in \p state.
    /// \returns progress span of the current state
    ProgressSpan update_progress_span(const State state) {
        if (!workflow_) {
            return ProgressSpan { 0, 0 };
        }

        const auto first_step = workflow_->steps().begin();
        const auto step = std::ranges::find_if(workflow_->steps(), [state](const auto &step) { return step.state == state; });

        if (step == workflow_->steps().end()) {
            // The state is not in the workflow -> just return the current progress
            return ProgressSpan { current_progress_, current_progress_ };
        }

        const auto step_index = step - first_step;
        const auto step_base_cumulative_scale = (step == first_step) ? 0 : std::prev(step)->cumulative_scale;

        if (step_index > current_step_index_ || current_step_index_ == invalid_step) {
            // If we've progressed in the workflow, consider the step progress end as the starting point
            // and scale the remaining steps between current progress and 100%
            // As a result, skipped steps in the workflow will not cause big jumps in the progress,
            // but the remaining progress will get redistributed

            // This will also be triggered for first update_progress call,
            // at which point the current_step_span_.max should be 0
            // - so we're starting the progress from the 0, whatever step we begin on.

            current_step_span_.min = current_step_span_.max;

            // step_span_.max will be recomputed below

        } else if (step_index < current_step_index_) {
            // If we've regressed in the workflow, "reset" the smart scaling and assume default workflow scales

            current_step_span_.min = static_cast<ProgressPercent>(std::roundf(float(step_base_cumulative_scale) / workflow_->scale_sum() * 100.0f));

            // step_span_.max will be recomputed below
        }

        current_step_index_ = step_index;

        const auto step_scale = step->cumulative_scale - step_base_cumulative_scale;

        /// Scale of all remaining steps (including this one)
        const auto remaining_scale = workflow_->scale_sum() - step_base_cumulative_scale;

        current_step_span_.max = current_step_span_.min + static_cast<int>(std::roundf(float(step_scale) / remaining_scale * (100 - current_step_span_.min)));

        return current_step_span_;
    }

private:
    const Workflow *workflow_ = nullptr;
};
