#include <yaml-cpp/yaml.h>

#include "runtime/optional.h"
#include "runtime/streams.h"
#include "runtime/critical_section.h"

#include "runtime/yaml.h"

// TODO: add key-arrays support
void yaml_node_to_mixed(const YAML::Node &node, mixed &data) {
  data.clear();
  if (node.IsNull()) {
    php_warning("Yaml node is null. Skipping it");
    return;
  }
  if (node.IsScalar()) {
    string string_data = string(node.as<std::string>().c_str());
    if (string_data.is_int()) {
      data = string_data.to_int();
      return;
    }
    dl::enter_critical_section();
    double *float_data = new double;
    if (string_data.try_to_float(float_data)) {
      data = *float_data;
    }
    else data = string_data;
    delete float_data;
    dl::leave_critical_section();
    return;
  }
  if (node.IsSequence()) {
    for (auto it = node.begin(); it != node.end(); ++it) {
      mixed data_piece;
      yaml_node_to_mixed(*it, data_piece);
      data.push_back(data_piece);
    }
  }
  else if (node.IsMap()) {
    for (auto it : node) {
      mixed data_piece;
      yaml_node_to_mixed(it.second, data_piece);
      data[string(it.first.as<std::string>().c_str())] = data_piece;
    }
  }
}

// TODO: rewrite using switch
// TODO: add key-arrays support
void mixed_to_yaml_node(const mixed &data, YAML::Node &node) {
  if (!data.is_array()) {
    if (data.is_null()) {
      php_warning("Cannot convert (mixed)null into yaml node");
      return;
    }
    if (data.is_string()) {
      node = std::string(data.as_string().c_str());
      return;
    }
    if (data.is_int()) {
      node = data.as_int();
      return;
    }
    if (data.is_float()) {
      node = data.as_double();
      return;
    }
    if (data.is_bool()) {
      if (!data.as_bool()) {
          node = false;
      }
      else {
          node = true;
      }
      return;
    }
  }
  array<mixed> data_array = data.as_array();
  if (data_array.is_pseudo_vector()) {
    for (auto it = data_array.begin(); it != data_array.end(); ++it) {
      mixed data_piece = it.get_value();
      if (data_piece.is_array()) {
        YAML::Node node_piece;
        mixed_to_yaml_node(data_piece, node_piece);
        node.push_back(node_piece);
      }
      else if (data_piece.is_string()) {
        node.push_back(std::string(data_piece.as_string().c_str()));
      }
      else if (data_piece.is_int()) {
        node.push_back(data_piece.as_int());
      }
      else if (data_piece.is_float()) {
        node.push_back(data_piece.as_double());
      }
      else if (data_piece.is_bool()) {
          node.push_back(data_piece.as_bool());
      }
      else if (data_piece.is_null()) {
        php_warning("Data piece is null. Skipping it");
      }
      else php_warning("Unknown data type. Skipping it"); // maybe this is redundant?
    }
  } else {
    for (auto it = data_array.begin(); it != data_array.end(); ++it) {
      mixed data_piece = it.get_value();
      YAML::Node node_piece;
      mixed_to_yaml_node(data_piece, node_piece);
      mixed data_key = it.get_key();
      if (data_key.is_null() || data_key.is_array()) {
        php_warning("Key is null or an array. Cannot handle it. Pushing data as in a vector");
        node.push_back(node_piece);
      }
      else if (data_key.is_string()) {
        node[std::string(data_key.as_string().c_str())] = node_piece;
      }
      else if (data_key.is_int()) { // float and bool are cast to int
        node[data_key.as_int()] = node_piece;
      }
      else { // maybe this is redundant?
        php_warning("Unknown key type. Pushing data as in a vector");
        node.push_back(node_piece);
      }
    }
  }
}

bool f$yaml_emit_file(const string &filename, const mixed &data) {
  if (filename.empty()) {
    php_warning("Filename cannot be empty");
    return false;
  }
  string emitted_data = f$yaml_emit(data);
  Optional<int64_t> size = f$file_put_contents(filename, emitted_data);
  if (size.is_false()) {
    php_warning("Error while writing to file \"%s\"", filename.c_str());
    return false;
  }
  return true;
}

string f$yaml_emit(const mixed &data) {
  YAML::Node node;
  mixed_to_yaml_node(data, node);
  return string(YAML::Dump(node).c_str());
}

mixed f$yaml_parse_file(const string &filename, int pos) {
  if (filename.empty()) {
    php_warning("Filename cannot be empty");
    return {};
  }
  Optional<string> data = f$file_get_contents(filename);
  if (data.is_false()) {
    php_warning("Error while reading file \"%s\"", filename.c_str());
    return {};
  }
  return f$yaml_parse(data.ref(), pos);
}

mixed f$yaml_parse(const string &data, int pos) {
  if (pos < 0) {
    php_warning("Argument \"pos\" = %d. Values other than 0 are not supported yet. Setting to default (pos = 0)", pos);
  }
  YAML::Node node = YAML::Load(data.c_str());
  mixed parsed_data;
  yaml_node_to_mixed(node, parsed_data);
  return parsed_data;
}
