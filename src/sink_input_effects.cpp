#include <giomm/settings.h>
#include "sink_input_effects.hpp"

SinkInputEffects::SinkInputEffects(std::shared_ptr<PulseManager> pulse_manager)
    : PipelineBase(pulse_manager->apps_sink_info->rate), pm(pulse_manager) {
    log_tag = "sie: ";

    set_pulseaudio_props(
        "application.id=com.github.wwmm.pulseeffects.sinkinputs");

    set_source_monitor_name(pm->apps_sink_info->monitor_source_name);

    auto PULSE_SINK = std::getenv("PULSE_SINK");

    if (PULSE_SINK) {
        set_output_sink_name(PULSE_SINK);
    } else {
        set_output_sink_name(pm->server_info.default_sink_name);
    }

    pm->sink_input_added.connect(
        sigc::mem_fun(*this, &SinkInputEffects::on_app_added));
    pm->sink_input_changed.connect(
        sigc::mem_fun(*this, &SinkInputEffects::on_app_changed));
    pm->sink_input_removed.connect(
        sigc::mem_fun(*this, &SinkInputEffects::on_app_removed));

    auto app_settings = Gio::Settings::create("com.github.wwmm.pulseeffects");

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

    // app_settings->bind("buffer-out", source, "buffer-time", flag);
    // app_settings->bind("latency-out", source, "latency-time", flag);
}

SinkInputEffects::~SinkInputEffects() {}
