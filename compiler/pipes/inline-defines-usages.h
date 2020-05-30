#pragma once

#include "compiler/compiler-core.h"
#include "compiler/function-pass.h"

class InlineDefinesUsagesPass final : public FunctionPassBase {
public:
  ClassPtr class_id;
  ClassPtr lambda_class_id;

  bool on_start(FunctionPtr function) override;

  string get_description() override {
    return "Inline defines pass";
  }

  VertexPtr on_enter_vertex(VertexPtr root) override;
};
