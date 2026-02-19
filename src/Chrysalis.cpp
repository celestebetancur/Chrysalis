#include "chuck_globals.h"
#include "plugin.hpp"
#include <chuck.h>
#include <iostream>
#include <osdialog.h>

using namespace rack;

#define DEBUG_LOG(format, ...)                                                 \
  fprintf(stderr, "[Chrysalis] " format "\n", ##__VA_ARGS__)

std::string compilationError = "";

// TODO: Make this local to each Chrystalis
void my_chout_cb(const char *msg) {
  // DEBUG_LOG("[chout] %s", msg);
  compilationError = msg;
}
void my_cherr_cb(const char *msg) {
  // DEBUG_LOG("[cherr] %s", msg);
  compilationError = msg;
}
void my_stdout_cb(const char *msg) {
  // DEBUG_LOG("[stdout] %s", msg);
  compilationError = msg;
}
void my_stderr_cb(const char *msg) {
  // DEBUG_LOG("[stderr] %s", msg);
  compilationError = msg;
}

struct Chrysalis : Module {
  enum ParamId { KNOB_1, KNOB_2, KNOB_3, KNOB_4, PARAMS_LEN };
  enum InputId { INPUT_1, INPUT_2, INPUT_3, INPUT_4, INPUTS_LEN };
  enum OutputId { OUTPUT_1, OUTPUT_2, OUTPUT_3, OUTPUT_4, OUTPUTS_LEN };
  enum LightId { LIGHTS_LEN };

  ::ChucK *the_chuck = nullptr;
  std::string currentFilePath = "";
  bool chuckReady = false;

  // Audio buffers for ChucK
  float *inBuffer = nullptr;
  float *outBuffer = nullptr;

  Chrysalis() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(KNOB_1, 0.f, 1.f, 0.f, "Knob 1");
    configParam(KNOB_2, 0.f, 1.f, 0.f, "Knob 2");
    configParam(KNOB_3, 0.f, 1.f, 0.f, "Knob 3");
    configParam(KNOB_4, 0.f, 1.f, 0.f, "Knob 4");
    configInput(INPUT_1, "adc 0");
    configInput(INPUT_2, "adc 1");
    configInput(INPUT_3, "adc 2");
    configInput(INPUT_4, "adc 3");
    configOutput(OUTPUT_1, "dac 0");
    configOutput(OUTPUT_2, "dac 1");
    configOutput(OUTPUT_3, "dac 2");
    configOutput(OUTPUT_4, "dac 3");

    // Initialize ChucK
    initChucK();

    std::string autoloadPath = asset::plugin(pluginInstance, "autoload.txt");
    FILE *f = fopen(autoloadPath.c_str(), "r");
    if (f) {
      char buf[1024];
      if (fgets(buf, sizeof(buf), f)) {
        std::string path(buf);
        // Trim newline
        size_t last = path.find_last_not_of(" \t\n\r");
        if (last != std::string::npos) {
          path = path.substr(0, last + 1);
        }
        if (!path.empty()) {
          // Check if path is absolute
          bool isAbsolute = false;
          if (path.size() > 0 && path[0] == '/')
            isAbsolute = true;
#ifdef _WIN32
          if (path.size() > 1 && path[1] == ':')
            isAbsolute = true;
#endif

          if (!isAbsolute) {
            path = asset::plugin(pluginInstance, path);
          }
          loadFile(path);
        }
      }
      fclose(f);
    }
  }

  ~Chrysalis() { cleanupChucK(); }

  void initChucK() {
    if (the_chuck)
      cleanupChucK();

    the_chuck = new ::ChucK();

    the_chuck->setParam(CHUCK_PARAM_SAMPLE_RATE,
                        (int)APP->engine->getSampleRate());
    the_chuck->setParam(CHUCK_PARAM_INPUT_CHANNELS, 4);
    the_chuck->setParam(CHUCK_PARAM_OUTPUT_CHANNELS, 4);

    // Initialize
    the_chuck->init();

    // Register callbacks
    the_chuck->setChoutCallback(my_chout_cb);
    the_chuck->setCherrCallback(my_cherr_cb);
    the_chuck->setStdoutCallback(my_stdout_cb);
    the_chuck->setStderrCallback(my_stderr_cb);

    the_chuck->start();

    // Allocate buffers
    // 4 channels * 1 sample
    inBuffer = new float[4];
    outBuffer = new float[4];

    chuckReady = true;
  }

  void cleanupChucK() {
    // DEBUG_LOG("Cleaning up ChucK instance...");
    chuckReady = false;
    if (the_chuck) {
      CK_SAFE_DELETE(the_chuck);
    }
    if (inBuffer)
      delete[] inBuffer;
    if (outBuffer)
      delete[] outBuffer;
    inBuffer = nullptr;
    outBuffer = nullptr;
  }

  void process(const ProcessArgs &args) override {
    if (!chuckReady || !the_chuck)
      return;

    // TODO: Check this values to make them correspond to VCV Rack
    inBuffer[0] = inputs[INPUT_1].getVoltage() / 5.0f;
    inBuffer[1] = inputs[INPUT_2].getVoltage() / 5.0f;
    inBuffer[2] = inputs[INPUT_3].getVoltage() / 5.0f;
    inBuffer[3] = inputs[INPUT_4].getVoltage() / 5.0f;

    the_chuck->run(inBuffer, outBuffer, 1);

    outputs[OUTPUT_1].setVoltage(clamp(outBuffer[0] * 1.0f, -10.0f, 10.0f));
    outputs[OUTPUT_2].setVoltage(clamp(outBuffer[1] * 1.0f, -10.0f, 10.0f));
    outputs[OUTPUT_3].setVoltage(clamp(outBuffer[2] * 1.0f, -10.0f, 10.0f));
    outputs[OUTPUT_4].setVoltage(clamp(outBuffer[3] * 1.0f, -10.0f, 10.0f));

    // Update global variables. Each Chrystalis instantiates a new ChucK - check
    // peformance
    if (the_chuck->globals()) {
      for (int i = 0; i < 4; i++) {
        std::string name = "Chrysalis_" + std::to_string(i + 1);
        the_chuck->globals()->setGlobalFloat(name.data(),
                                             params[KNOB_1 + i].getValue());
      }
    }
  }

  void loadFile(std::string path) {
    if (!chuckReady)
      return;

    the_chuck->removeAllShreds();
    currentFilePath = path;
    the_chuck->compileFile(path, "", 1);
    compilationError = "";
  }

  void reloadFile() {
    if (!chuckReady)
      return;

    the_chuck->removeAllShreds();
    the_chuck->compileFile(currentFilePath, "", 1);
    compilationError = "";
  }
};

struct ChucKWidget : ModuleWidget {
  Label *fileLabel;

  ChucKWidget(Chrysalis *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Chrysalis.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(
        createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(
        Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Knobs (Top)
    addParam(createParamCentered<RoundSmallBlackKnob>(Vec(20, 145), module,
                                                      Chrysalis::KNOB_1));
    addParam(createParamCentered<RoundSmallBlackKnob>(Vec(50, 145), module,
                                                      Chrysalis::KNOB_2));
    addParam(createParamCentered<RoundSmallBlackKnob>(Vec(80, 145), module,
                                                      Chrysalis::KNOB_3));
    addParam(createParamCentered<RoundSmallBlackKnob>(Vec(110, 145), module,
                                                      Chrysalis::KNOB_4));

    // Inputs (Left Column)
    addInput(createInputCentered<PJ301MPort>(Vec(25, 180), module,
                                             Chrysalis::INPUT_1));
    addInput(createInputCentered<PJ301MPort>(Vec(25, 215), module,
                                             Chrysalis::INPUT_2));
    addInput(createInputCentered<PJ301MPort>(Vec(25, 250), module,
                                             Chrysalis::INPUT_3));
    addInput(createInputCentered<PJ301MPort>(Vec(25, 285), module,
                                             Chrysalis::INPUT_4));

    // Outputs (Right Column)
    addOutput(createOutputCentered<PJ301MPort>(Vec(65, 180), module,
                                               Chrysalis::OUTPUT_1));
    addOutput(createOutputCentered<PJ301MPort>(Vec(65, 215), module,
                                               Chrysalis::OUTPUT_2));
    addOutput(createOutputCentered<PJ301MPort>(Vec(65, 250), module,
                                               Chrysalis::OUTPUT_3));
    addOutput(createOutputCentered<PJ301MPort>(Vec(65, 285), module,
                                               Chrysalis::OUTPUT_4));

    // File Label
    fileLabel = createWidget<Label>(Vec(5, 35));
    fileLabel->text = "(No file loaded)";
    fileLabel->fontSize = 11.0f;
    fileLabel->color = nvgRGB(0x60, 0x60, 0x60);

    if (module && !module->currentFilePath.empty()) {
      std::string path = module->currentFilePath;
      size_t found = path.find_last_of("/\\");
      fileLabel->text =
          (found != std::string::npos) ? path.substr(found + 1) : path;
    }
    addChild(fileLabel);

    // Load Button
    LoadButton *btn = createWidget<LoadButton>(Vec(10, 57));
    btn->text = " +  .ck";
    btn->box.size = Vec(box.size.x - 50, 24);
    btn->module = module;
    btn->label = fileLabel;
    addChild(btn);

    // Reload Button
    ReloadButton *reloadBtn =
        createWidget<ReloadButton>(Vec(box.size.x - 35, 57));
    reloadBtn->text = "=";
    reloadBtn->box.size = Vec(25, 24);
    reloadBtn->module = module;
    addChild(reloadBtn);

    // Status Label
    StatusLabel *statusLbl = createWidget<StatusLabel>(Vec(5, 80));
    statusLbl->text = "Status: ...";
    statusLbl->fontSize = 8.0f;
    statusLbl->box.size = Vec(box.size.x - 20, 15);
    statusLbl->module = module;
    statusLbl->color = nvgRGB(0x88, 0x88, 0x88);
    addChild(statusLbl);
  }

  struct LoadButton : Button {
    Chrysalis *module;
    Label *label;

    void onAction(const event::Action &e) override {
      if (!module)
        return;

      char *pathRaw = osdialog_file(OSDIALOG_OPEN, nullptr, nullptr, nullptr);
      if (pathRaw) {
        std::string path = pathRaw;
        module->loadFile(path);
        if (label) {
          size_t found = path.find_last_of("/\\");
          label->text =
              (found != std::string::npos) ? path.substr(found + 1) : path;
        }
        free(pathRaw);
      }
    }
  };

  struct ReloadButton : Button {
    Chrysalis *module;

    void onAction(const event::Action &e) override {
      if (!module || module->currentFilePath.empty())
        return;
      module->reloadFile();
    }
  };

  struct StatusLabel : Label {
    Chrysalis *module;
    int frame = 0;

    void step() override {
      Label::step();
      if (!module || !module->the_chuck || ++frame % 60 != 0)
        return;

      // Update status text
      if (module->chuckReady && module->the_chuck->vm()) {

        t_CKTIME now = module->the_chuck->now();
        t_CKUINT shredID = module->the_chuck->vm()->last_id();

        char buf[128];
        snprintf(buf, sizeof(buf), "Now: %.2f \nShred ID: %zu \n[ChucK] %s",
                 now, shredID, compilationError.data());
        text = buf;
      } else {
        text = "VM Not Ready";
      }
    }
  };
};

Model *modelChrysalis = createModel<Chrysalis, ChucKWidget>("Chrysalis");
