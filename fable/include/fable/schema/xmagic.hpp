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
 * \file fable/schema/xmagic.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 *
 * This file defines the `make_prototype` functions that can automatically
 * derive the prototype via the `make_schema` functions. It also contains the
 * definitions of constructors that make use of these functions.
 *
 * Unfortunately, all `make_schema` functions need to already be defined at the
 * point of definition of these constructors, which is why they have to be in
 * this file in the first place.
 *
 * So we declare the constructors in their relative class definitions, and
 * we define them here. This file is then included after all other files.
 * And then it works.
 */

#pragma once

#include <map>      // for map<>
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector<>
#include <array>    // for array<>

#include <fable/fable_fwd.hpp>        // for Confable
#include <fable/schema/array.hpp>     // for Array<>
#include <fable/schema/confable.hpp>  // for FromConfable<>
#include <fable/schema/const.hpp>     // for Const<>
#include <fable/schema/map.hpp>       // for Map<>
#include <fable/schema/optional.hpp>  // for Optional<>
#include <fable/schema/vector.hpp>    // for Vector<>

namespace fable {
namespace schema {

template <typename T, typename P>
Vector<T, P>::Vector(std::vector<T>* ptr, std::string desc)
    : Vector<T, P>(ptr, make_prototype<T>(), std::move(desc)) {}

template <typename T, typename S>
Vector<T, decltype(make_prototype<T>())> make_schema(std::vector<T>* ptr, S&& desc) {
  return Vector<T, decltype(make_prototype<T>())>(ptr, std::forward<S>(desc));
}

template <typename T, size_t N, typename P>
Array<T, N, P>::Array(std::array<T, N>* ptr, std::string&& desc)
    : Array<T, N, P>(ptr, make_prototype<T>(), std::move(desc)) {}

template <typename T, size_t N, typename S>
Array<T, N, decltype(make_prototype<T>())> make_schema(std::array<T, N>* ptr, S&& desc) {
  return Array<T, N, decltype(make_prototype<T>())>(ptr, std::forward<S>(desc));
}

template <typename T, typename P>
Const<T, P>::Const(const T& constant, std::string desc)
    : Const<T, P>(constant, make_prototype<T>(), std::move(desc)) {}

template <typename T, typename S>
Const<T, decltype(make_prototype<T>())> make_const_schema(const T& constant, S&& desc) {
  return Const<T, decltype(make_prototype<T>())>(constant, std::forward<S>(desc));
}

template <typename T, typename P>
Map<T, P>::Map(std::map<std::string, T>* ptr, std::string desc)
    : Map<T, P>(ptr, make_prototype<T>(), std::move(desc)) {}

template <typename T, typename S>
Map<T, decltype(make_prototype<T>())> make_schema(std::map<std::string, T>* ptr,
                                                  S&& desc) {
  return Map<T, decltype(make_prototype<T>())>(ptr, std::forward<S>(desc));
}

template <typename T, typename P>
Optional<T, P>::Optional(T* ptr, std::string desc)
    : Optional<T, P>(ptr, make_prototype<typename T::value_type>(), std::move(desc)) {}

template <typename T, typename S, std::enable_if_t<is_optional<T>::value, bool> = true>
Optional<T, decltype(make_prototype<typename T::value_type>())> make_schema(T* ptr, S&& desc) {
  return Optional<T, decltype(make_prototype<typename T::value_type>())>(ptr, std::forward<S>(desc));
}

template <typename T, typename S, std::enable_if_t<std::is_base_of_v<Confable, T>, int>>
auto make_prototype(S&& desc) {
  return FromConfable<T>(std::forward<S>(desc));
}

template <typename T, typename S, std::enable_if_t<!std::is_base_of_v<Confable, T>, int>>
auto make_prototype(S&& desc) {
  return make_schema(static_cast<T*>(nullptr), std::forward<S>(desc));
}

}  // namespace schema
}  // namespace fable