#include "SimStudentHumanAnimationsPlugin.h"

#include <model/AnimationModel.h>
#include <model/ModelFactoryRegistry.h>
#include <plugin/IModelPluginService.h>
#include <plugin/IPluginServices.h>
#include <plugin/PluginContext.h>

#include <cmath>
#include <string>
#include <unordered_set>

namespace student::n8ro::humananimations {
namespace {

using arkheon::astsim::AnimationModelInput;
using arkheon::astsim::AnimationModelOutput;

[[nodiscard]] bool hasJoint(
    const std::unordered_set<std::string>& availableJointIds,
    const char* jointId) {
    if (!jointId || *jointId == '\0') {
        return false;
    }
    if (availableJointIds.empty()) {
        return true;
    }
    return availableJointIds.find(jointId) != availableJointIds.end();
}

[[nodiscard]] std::unordered_set<std::string> collectJointIds(
    const AnimationModelInput& input) {
    std::unordered_set<std::string> availableJointIds;
    availableJointIds.reserve(input.entity.joints.size());
    for (const auto& joint : input.entity.joints) {
        availableJointIds.insert(joint.jointId);
    }
    return availableJointIds;
}

void addIfAvailable(
    const std::unordered_set<std::string>& availableJointIds,
    AnimationModelOutput& output,
    const char* jointId,
    double xRad,
    double yRad,
    double zRad) {
    if (hasJoint(availableJointIds, jointId)) {
        output.jointOverrides.push_back({jointId, xRad, yRad, zRad});
    }
}

[[nodiscard]] double smoothCycle(double t, double speed, double phase = 0.0) {
    return 0.5 - 0.5 * std::cos(t * speed + phase);
}

[[nodiscard]] bool evaluateRightArmWave(
    const AnimationModelInput& input,
    AnimationModelOutput& output) {
    const auto availableJointIds = collectJointIds(input);
    const double t = input.simulationTimeSeconds;
    const double wave = std::sin(t * 5.5);
    const double elbowWave = std::sin(t * 6.8 + 0.8);

    output.clearExistingJointOverrides = true;
    output.jointOverrides.clear();
    addIfAvailable(availableJointIds, output, "rightShoulder", 1.35, -0.25 + wave * 0.10, 1.05 + wave * 0.22);
    addIfAvailable(availableJointIds, output, "rightElbow", 1.55 + elbowWave * 0.18, 0.0, 0.55 + wave * 0.28);
    addIfAvailable(availableJointIds, output, "leftShoulder", 0.30, 0.08, -0.45);
    addIfAvailable(availableJointIds, output, "leftElbow", 0.45, 0.0, -0.18);
    return !output.jointOverrides.empty();
}

[[nodiscard]] bool evaluateWalkInPlace(
    const AnimationModelInput& input,
    AnimationModelOutput& output) {
    const auto availableJointIds = collectJointIds(input);
    const double t = input.simulationTimeSeconds;
    const double step = std::sin(t * 4.0);
    const double leftLift = std::max(0.0, step);
    const double rightLift = std::max(0.0, -step);
    const double leftBack = std::max(0.0, -step);
    const double rightBack = std::max(0.0, step);
    const double armSwing = std::sin(t * 4.0);

    output.clearExistingJointOverrides = true;
    output.jointOverrides.clear();
    addIfAvailable(availableJointIds, output, "leftHip", -0.15 - leftLift * 0.35 + leftBack * 0.25, 0.0, 0.08);
    addIfAvailable(availableJointIds, output, "rightHip", -0.15 - rightLift * 0.35 + rightBack * 0.25, 0.0, -0.08);
    addIfAvailable(availableJointIds, output, "leftKnee", leftLift * 0.85, 0.0, 0.0);
    addIfAvailable(availableJointIds, output, "rightKnee", rightLift * 0.85, 0.0, 0.0);
    addIfAvailable(availableJointIds, output, "leftAnkle", -leftLift * 0.30, 0.0, 0.0);
    addIfAvailable(availableJointIds, output, "rightAnkle", -rightLift * 0.30, 0.0, 0.0);
    addIfAvailable(availableJointIds, output, "leftShoulder", 0.40, armSwing * 0.18, -0.55 - armSwing * 0.20);
    addIfAvailable(availableJointIds, output, "rightShoulder", 0.40, -armSwing * 0.18, 0.55 - armSwing * 0.20);
    addIfAvailable(availableJointIds, output, "leftElbow", 0.55, 0.0, -0.20);
    addIfAvailable(availableJointIds, output, "rightElbow", 0.55, 0.0, 0.20);
    return !output.jointOverrides.empty();
}

[[nodiscard]] bool evaluateBothArmsRaise(
    const AnimationModelInput& input,
    AnimationModelOutput& output) {
    const auto availableJointIds = collectJointIds(input);
    const double t = input.simulationTimeSeconds;
    const double lift = smoothCycle(t, 3.0);
    const double shoulder = 0.70 + lift * 1.15;
    const double elbow = 0.55 + lift * 0.55;
    const double sway = std::sin(t * 3.0) * 0.12;

    output.clearExistingJointOverrides = true;
    output.jointOverrides.clear();
    addIfAvailable(availableJointIds, output, "leftShoulder", shoulder, 0.25 + sway, -1.05);
    addIfAvailable(availableJointIds, output, "rightShoulder", shoulder, -0.25 - sway, 1.05);
    addIfAvailable(availableJointIds, output, "leftElbow", elbow, 0.0, -0.30);
    addIfAvailable(availableJointIds, output, "rightElbow", elbow, 0.0, 0.30);
    return !output.jointOverrides.empty();
}

} // namespace

int SimStudentHumanAnimationsPlugin::getInterfaceVersion() const {
    return 1;
}

arkheon::astlib::PluginMetadata SimStudentHumanAnimationsPlugin::getMetadata() const {
    arkheon::astlib::PluginMetadata metadata;
    metadata.setPluginId("sim-student-human-animations");
    metadata.setVersion("1.0.0");
    metadata.setAuthor("Student");
    return metadata;
}

void SimStudentHumanAnimationsPlugin::initialize(arkheon::astlib::PluginContext& context) {
    initialized_ = true;
    shutdown_ = false;
    registeredAnimationCodes_.clear();
    modelFactoryRegistry_ = nullptr;

    if (context.services) {
        auto* rawService = context.services->getService(arkheon::astsim::IModelPluginService::kPluginServiceId);
        auto* service = static_cast<arkheon::astsim::IModelPluginService*>(rawService);
        modelFactoryRegistry_ = service ? &service->modelFactoryRegistry() : nullptr;
    }

    if (!modelFactoryRegistry_) {
        return;
    }

    auto* prototypeBase = modelFactoryRegistry_->getRegisteredPrototype(modelType_);
    auto* prototypeAnimationModel = dynamic_cast<arkheon::astsim::IAnimationModel*>(prototypeBase);
    if (!prototypeAnimationModel) {
        return;
    }

    const struct {
        const char* code;
        arkheon::astsim::IAnimationModel::AnimationEvaluationFunction evaluator;
    } animations[] = {
        {"Right Arm Wave", evaluateRightArmWave},
        {"Walk In Place", evaluateWalkInPlace},
        {"Both Arms Raise", evaluateBothArmsRaise},
    };

    for (const auto& animation : animations) {
        if (prototypeAnimationModel->registerAnimation(animation.code, animation.evaluator)) {
            registeredAnimationCodes_.push_back(animation.code);
        }
    }
}

void SimStudentHumanAnimationsPlugin::tick(double dt) {
    static_cast<void>(dt);
    if (!initialized_ || shutdown_) {
        return;
    }
}

void SimStudentHumanAnimationsPlugin::shutdown() {
    if (modelFactoryRegistry_) {
        auto* prototypeBase = modelFactoryRegistry_->getRegisteredPrototype(modelType_);
        auto* prototypeAnimationModel = dynamic_cast<arkheon::astsim::IAnimationModel*>(prototypeBase);
        if (prototypeAnimationModel) {
            for (const auto& code : registeredAnimationCodes_) {
                static_cast<void>(prototypeAnimationModel->registerAnimation(
                    code,
                    arkheon::astsim::IAnimationModel::AnimationEvaluationFunction {}));
            }
        }
    }

    registeredAnimationCodes_.clear();
    shutdown_ = true;
    modelFactoryRegistry_ = nullptr;
}

} // namespace student::n8ro::humananimations

extern "C" {

ARKHEON_ASTLIB_API arkheon::astlib::IPlugin* create_plugin() {
    return new student::n8ro::humananimations::SimStudentHumanAnimationsPlugin();
}

ARKHEON_ASTLIB_API void destroy_plugin(arkheon::astlib::IPlugin* plugin) {
    if (plugin) {
        delete plugin;
    }
}

ARKHEON_ASTLIB_API const char* get_plugin_signature() {
    return "ARKHEON_PLUGIN_V1";
}

} // extern "C"
