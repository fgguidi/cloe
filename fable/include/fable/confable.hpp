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
 * \file fable/confable.hpp
 * \see  fable/confable.cpp
 * \see  fable/schema.hpp
 */

#pragma once

#include <memory>    // for unique_ptr<>
#include <optional>  // for optional<>

#include <fable/conf.hpp>       // for Conf, Json
#include <fable/fable_fwd.hpp>  // for Schema, SchemaError

#define CONFABLE_FRIENDS(xType)                                           \
  using ::fable::Confable::to_json;                                       \
  friend void to_json(::fable::Json& j, const xType& t) { t.to_json(j); } \
  friend void from_json(const ::fable::Json& j, xType& t) { t.from_conf(::fable::Conf{j}); }

#define CONFABLE_SCHEMA(xType) \
  CONFABLE_FRIENDS(xType)      \
  ::fable::Schema schema_impl() override

namespace fable {

/**
 * Confable is a base-class that you can inherit for your own types to
 * provide native fable integration.
 *
 * This provides easy deserialization with validation
 * and serialization, primarily by providing the schema_impl() definition.
 *
 * Example:
 *
 *     class Foo : public fable::Confable {
 *       double bar;
 *
 *       fable::Schema schema_impl() override {
 *         return fable::Schema{
 *           {"bar", fable::make_schema(&bar, "bar description")},
 *         };
 *       }
 *
 *       using fable::Confable::to_json;
 *       friend void to_json(::fable::Json& j, const xType& t) {
 *         t.to_json(j);
 *       }
 *       friend void from_json(const ::fable::Json& j, xType& t) {
 *         t.from_conf(::fable::Conf{j});
 *       }
 *     };
 *
 * In order to eliminate some of this boilerplate, the following macro simplifies
 * things and keeps it short and to the point:
 *
 *     class Foo : public fable::Confable {
 *       double bar;
 *
 *       CONFABLE_SCHEMA(Foo) {
 *         using namespace fable;
 *         return Schema{
 *           {"bar", make_schema(&bar, "bar description")},
 *         };
 *       }
 *     };
 *
 * Your class now be used with `Schema` and `make_schema` definitions, and it
 * just works:
 *
 *     class ExtraFine : public fable::Confable {
 *       Foo foo;
 *
 *       CONFABLE_SCHEMA(ExtraFine) {
 *         using namespace fable;
 *         return Schema{
 *           {"foo", make_schema(&foo, "foo description")},
 *         };
 *       }
 *     };
 */
class Confable {
 public:
  Confable() noexcept = default;
  Confable(const Confable& /*unused*/) noexcept;
  Confable(Confable&&) noexcept = default;
  Confable& operator=(const Confable& other) noexcept;
  Confable& operator=(Confable&& other) noexcept;

  virtual ~Confable() noexcept = default;

  /**
   * Reset the internal schema cache.
   *
   * This causes schema_impl to be called next time the schema is requested.
   */
  virtual void reset_schema();

  /**
   * Return the object schema for deserialization.
   *
   * This method uses `schema_impl` under the hood, which the object should
   * implement.
   */
  [[nodiscard]] Schema& schema();

  /**
   * Return the object schema for description and validation.
   *
   * This method uses `schema_impl` under the hood, which the object should
   * implement.
   */
  [[nodiscard]] const Schema& schema() const;

  /**
   * Validate a Conf without applying it.
   *
   * By default, this uses the validate_or_throw() method on schema(). If you want to
   * guarantee anything extending beyond what's possible with schema, you can
   * do that here.
   *
   * This method should NOT call from_conf without also overriding from_conf
   * to prevent infinite recursion.
   */
  virtual void validate_or_throw(const Conf& c) const;

  /**
   * Validate a Conf without applying it and without throwing.
   *
   * By default, this uses the validate() method on schema(). If you want to
   * guarantee anything extending beyond what's possible with schema, you can
   * do that here.
   *
   * This method should NOT call from_conf without also overriding from_conf
   * to prevent infinite recursion.
   *
   * This method should not reset err unless the method returns false.
   *
   * \param c JSON to validate
   * \param err reference to store error in
   * \return true if validate successful
   */
  virtual bool validate(const Conf& c, std::optional<SchemaError>& err) const;

  /**
   * Deserialize a Confable from a Conf.
   *
   * Unless you have special needs, it is recommended to implement
   *
   *     Schema schema_impl()
   *
   * and use the default implementation of this.
   */
  virtual void from_conf(const Conf& c);

  /**
   * Serialize a Confable to Json.
   *
   * Note: If you implement this type, make sure to either use CONFABLE_FRIENDS
   * or a using Confable::to_json statement in your derived type.
   */
  virtual void to_json(Json& j) const;

  /**
   * Serialize a Confable to Json.
   */
  [[nodiscard]] Json to_json() const;

 protected:
  /**
   * Return a new instance of the schema for this object.
   *
   * This schema is then stored in schema_. This is called every time the
   * object is created or moved, since Schema contains references to fields
   * that (should be) in the object.
   */
  [[nodiscard]] virtual Schema schema_impl();

 private:
  mutable std::unique_ptr<Schema> schema_;
};

}  // namespace fable

namespace nlohmann {

template <>
struct adl_serializer<fable::Confable> {
  static void to_json(json& j, const fable::Confable& c);
  static void from_json(const json& j, fable::Confable& c);
};

}  // namespace nlohmann
