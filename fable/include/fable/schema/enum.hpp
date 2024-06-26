/*
 * Copyright 2020 Robert Bosch GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * \file fable/schema/enum.hpp
 * \see  fable/schema.cpp
 * \see  fable/schema_test.cpp
 */

#pragma once

#include <map>          // for map<>
#include <string>       // for string
#include <type_traits>  // for enable_if_t<>, is_enum<>
#include <utility>      // for move, make_pair
#include <vector>       // for vector<>

#include <fable/enum.hpp>
#include <fable/schema/interface.hpp>  // for Base<>

namespace fable::schema {

template <typename T>
class Enum : public Base<Enum<T>> {
 public:  // Types and Constructors
  using Type = T;

  Enum(Type* ptr, std::string desc)
      : Base<Enum<T>>(JsonType::string, std::move(desc))
      , mapping_to_(enum_serialization<T>())
      , mapping_from_(enum_deserialization<T>())
      , ptr_(ptr) {
    keys_.reserve(mapping_to_.size());
    for (auto const& kv : mapping_to_) {
      keys_.emplace_back(kv.second);
    }
  }

 public:  // Special
  [[nodiscard]] const std::vector<std::string>& keys() const { return keys_; }

 public:  // Overrides
  [[nodiscard]] Json json_schema() const override {
    Json j{
        {"type", this->type_string()},
        {"enum", keys_},
    };
    this->augment_schema(j);
    return j;
  }

  bool validate(const Conf& c, std::optional<SchemaError>& err) const override {
    auto s = c.get<std::string>();
    if (mapping_from_.count(s) == 0) {
      return this->set_error(err, c, "invalid value for enum: {}", s);
    }
    return true;
  }

  using Interface::to_json;

  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    j = serialize(*ptr_);
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    *ptr_ = deserialize(c);
  }

  Json serialize(const Type& x) const { return mapping_to_.at(x); }

  Type deserialize(const Conf& c) const {
    auto s = c.get<std::string>();
    try {
      return mapping_from_.at(s);
    } catch (std::out_of_range& e) {
      throw this->error(c, "invalid value for enum: {}", s);
    }
  }

  void serialize_into(Json& j, const Type& x) const { j = mapping_to_.at(x); }

  void deserialize_into(const Conf& c, Type& x) const { x = deserialize(c); }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  const std::map<T, std::string>& mapping_to_;    // NOLINT: type-specific global ref
  const std::map<std::string, T>& mapping_from_;  // NOLINT: type-specific global ref
  std::vector<std::string> keys_;
  Type* ptr_{nullptr};
};

template <typename T, typename S, std::enable_if_t<std::is_enum_v<T>, int> = 0>
Enum<T> make_schema(T* ptr, S&& desc) {
  return {ptr, std::forward<S>(desc)};
}

}  // namespace fable::schema
