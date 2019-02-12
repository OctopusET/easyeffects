#include "spectrum_settings_ui.hpp"
#include "util.hpp"

SpectrumSettingsUi::SpectrumSettingsUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& builder,
    Application* application)
    : Gtk::Grid(cobject),
      settings(Gio::Settings::create("com.github.wwmm.pulseeffects.spectrum")),
      app(application) {
  // loading glade widgets

  builder->get_widget("show", show);
  builder->get_widget("fill", fill);
  builder->get_widget("show_bar_border", show_bar_border);
  builder->get_widget("spectrum_color_button", spectrum_color_button);
  builder->get_widget("background_color_button", background_color_button);
  builder->get_widget("use_custom_color", use_custom_color);

  get_object(builder, "n_points", n_points);
  get_object(builder, "height", height);
  get_object(builder, "scale", scale);
  get_object(builder, "exponent", exponent);
  get_object(builder, "sampling_freq", sampling_freq);
  get_object(builder, "line_width", line_width);

  // signals connection

  connections.push_back(
      settings->signal_changed("color").connect([&](auto key) {
        Glib::Variant<std::vector<double>> v;

        settings->get_value("color", v);

        auto rgba = v.get();

        Gdk::RGBA color;

        color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);

        spectrum_color_button->set_rgba(color);
      }));

  connections.push_back(
      settings->signal_changed("background-color").connect([&](auto key) {
        Glib::Variant<std::vector<double>> v;

        settings->get_value("background-color", v);

        auto rgba = v.get();

        Gdk::RGBA color;

        color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);

        background_color_button->set_rgba(color);
      }));

  show->signal_state_set().connect(
      sigc::mem_fun(*this, &SpectrumSettingsUi::on_show_spectrum), false);

  spectrum_color_button->signal_color_set().connect([&]() {
    auto spectrum_color = spectrum_color_button->get_rgba();

    auto v = Glib::Variant<std::vector<double>>::create(std::vector<double>{
        spectrum_color.get_red(), spectrum_color.get_green(),
        spectrum_color.get_blue(), spectrum_color.get_alpha()});

    settings->set_value("color", v);
  });

  background_color_button->signal_color_set().connect([&]() {
    auto color = background_color_button->get_rgba();

    auto v = Glib::Variant<std::vector<double>>::create(
        std::vector<double>{color.get_red(), color.get_green(),
                            color.get_blue(), color.get_alpha()});

    settings->set_value("background-color", v);
  });

  use_custom_color->signal_state_set().connect(
      sigc::mem_fun(*this, &SpectrumSettingsUi::on_use_custom_color), false);

  sampling_freq->signal_value_changed().connect(
      sigc::mem_fun(*this, &SpectrumSettingsUi::on_spectrum_sampling_freq_set),
      false);

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("show", show, "active", flag);

  settings->bind("fill", fill, "active", flag);
  settings->bind("show-bar-border", show_bar_border, "active", flag);
  settings->bind("n-points", n_points.get(), "value", flag);
  settings->bind("height", height.get(), "value", flag);
  settings->bind("scale", scale.get(), "value", flag);
  settings->bind("exponent", exponent.get(), "value", flag);
  settings->bind("sampling-freq", sampling_freq.get(), "value", flag);
  settings->bind("line-width", line_width.get(), "value", flag);
  settings->bind("use-custom-color", use_custom_color, "active", flag);
  settings->bind("use-custom-color", spectrum_color_button, "sensitive", flag);
  settings->bind("use-custom-color", background_color_button, "sensitive",
                 flag);
}

SpectrumSettingsUi::~SpectrumSettingsUi() {
  for (auto c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

void SpectrumSettingsUi::add_to_stack(Gtk::Stack* stack, Application* app) {
  auto builder = Gtk::Builder::create_from_resource(
      "/com/github/wwmm/pulseeffects/ui/spectrum_settings.glade");

  SpectrumSettingsUi* ui;

  builder->get_widget_derived("widgets_grid", ui, app);

  stack->add(*ui, "settings_spectrum", _("Spectrum"));
}

bool SpectrumSettingsUi::on_show_spectrum(bool state) {
  if (state) {
    app->sie->enable_spectrum();
    app->soe->enable_spectrum();
  } else {
    app->sie->disable_spectrum();
    app->soe->disable_spectrum();
  }

  return false;
}

bool SpectrumSettingsUi::on_use_custom_color(bool state) {
  if (state) {
    Glib::Variant<std::vector<double>> v;

    settings->get_value("color", v);

    auto rgba = v.get();

    Gdk::RGBA color;

    color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);

    spectrum_color_button->set_rgba(color);

    // background color

    settings->get_value("background-color", v);

    rgba = v.get();

    color.set_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);

    background_color_button->set_rgba(color);
  }

  return false;
}

void SpectrumSettingsUi::on_spectrum_sampling_freq_set() {
  app->sie->update_spectrum_interval(sampling_freq->get_value());
  app->soe->update_spectrum_interval(sampling_freq->get_value());
}
