/**
 * @file fluency.hpp
 * @author Alic Szecsei
 * @date 6/6/2023
 */

#pragma once

#include "../target/fluency_ffi.hpp"

#include <optional>
#include <vector>

// TODO: Smart pointers?

namespace fluency {
    class FluencyArgs {
    private:
        ffi::FluencyArgs* m_args = nullptr;

    public:
        FluencyArgs() { ffi::create_args(&m_args); }

        FluencyArgs(const FluencyArgs&) = delete; // copy constructor

        FluencyArgs(FluencyArgs&& other) noexcept
            : m_args(std::exchange(other.m_args, nullptr)) { } // move constructor

        auto operator=(const FluencyArgs&) -> FluencyArgs& = delete; // copy assignment

        auto operator=(FluencyArgs&& other) noexcept -> FluencyArgs& { // move assignment
            std::swap(m_args, other.m_args);
            return *this;
        }

        ~FluencyArgs() { ffi::destroy_args(m_args); }

        inline void set(std::string_view key, uint8_t value) {
            ffi::set_arg_u8(m_args, key.data(), value);
        }

        inline void set(std::string_view key, uint16_t value) {
            ffi::set_arg_u16(m_args, key.data(), value);
        }

        inline void set(std::string_view key, uint32_t value) {
            ffi::set_arg_u32(m_args, key.data(), value);
        }

        inline void set(std::string_view key, uint64_t value) {
            ffi::set_arg_u64(m_args, key.data(), value);
        }

        inline void set(std::string_view key, int8_t value) {
            ffi::set_arg_i8(m_args, key.data(), value);
        }

        inline void set(std::string_view key, int16_t value) {
            ffi::set_arg_i16(m_args, key.data(), value);
        }

        inline void set(std::string_view key, int32_t value) {
            ffi::set_arg_i32(m_args, key.data(), value);
        }

        inline void set(std::string_view key, int64_t value) {
            ffi::set_arg_i64(m_args, key.data(), value);
        }

        inline void set(std::string_view key, float value) {
            ffi::set_arg_f32(m_args, key.data(), value);
        }

        inline void set(std::string_view key, double value) {
            ffi::set_arg_f64(m_args, key.data(), value);
        }

        inline void set(std::string_view key, std::string_view value) {
            ffi::set_arg_string(m_args, key.data(), value.data());
        }

        inline void set_try_number(std::string_view key, std::string_view value) {
            ffi::set_arg_try_number(m_args, key.data(), value.data());
        }

        friend class FluencyBundle;
    };

    class FluencyAttribute {
    private:
        ffi::FluencyAttribute* m_attribute = nullptr;

        explicit FluencyAttribute(ffi::FluencyAttribute* attr)
            : m_attribute(attr) { }

    public:
        ~FluencyAttribute() { ffi::destroy_attribute(m_attribute); }

        auto get_key() -> std::string {
            char* key_c = nullptr;
            ffi::get_attribute_id(m_attribute, &key_c);
            std::string key(key_c);
            ffi::destroy_string(key_c);
            return key;
        }

        friend class FluencyMessage;
        friend class FluencyBundle;
    };

    class FluencyMessage {
    private:
        ffi::FluencyMessage* m_message = nullptr;

        explicit FluencyMessage(ffi::FluencyMessage* msg)
            : m_message(msg) { }

    public:
        ~FluencyMessage() { ffi::destroy_message(m_message); }

        auto get_attribute(std::string_view key) -> FluencyAttribute {
            ffi::FluencyAttribute* attr = nullptr;
            ffi::get_attribute(m_message, key.data(), &attr);
            return FluencyAttribute(attr);
        }

        // TODO: get_attributes

        friend class FluencyBundle;
    };

    class FluencyBundle {
    private:
        ffi::FluencyBundle* m_bundle = nullptr;

    public:
        explicit FluencyBundle(std::string_view locale) {
            auto result = ffi::create_bundle(locale.data(), &m_bundle);
            if (result != ffi::FluencyResult::Ok) { throw std::invalid_argument("Invalid locale"); }
        }

        FluencyBundle(const FluencyBundle&) = delete; // copy constructor

        FluencyBundle(FluencyBundle&& other) noexcept
            : m_bundle(std::exchange(other.m_bundle, nullptr)) { } // move constructor

        auto operator=(const FluencyBundle&) -> FluencyBundle& = delete; // copy assignment

        auto operator=(FluencyBundle&& other) noexcept -> FluencyBundle& { // move assignment
            std::swap(m_bundle, other.m_bundle);
            return *this;
        }

        ~FluencyBundle() { ffi::destroy_bundle(m_bundle); }

        auto add_resource(std::string_view string, std::vector<std::string>& errors) -> bool {
            char** errors_c = nullptr;
            size_t error_length = 0;
            auto result = ffi::add_resource(m_bundle, string.data(), &errors_c, &error_length);
            if (result != ffi::FluencyResult::Ok) { return false; }

            errors.clear();
            if (error_length != 0) {
                for (auto i = 0; i < error_length; ++i) { errors.emplace_back(errors_c[i]); }
                ffi::destroy_str_array(errors_c, error_length);
            }

            return true;
        }

        auto add_resource_overriding(std::string_view string, std::vector<std::string>& errors)
            -> bool {
            char** errors_c = nullptr;
            size_t error_length = 0;
            auto result =
                ffi::add_resource_overriding(m_bundle, string.data(), &errors_c, &error_length);
            if (result != ffi::FluencyResult::Ok) { return false; }

            errors.clear();
            if (error_length != 0) {
                for (auto i = 0; i < error_length; ++i) { errors.emplace_back(errors_c[i]); }
                ffi::destroy_str_array(errors_c, error_length);
            }

            return true;
        }

        void set_use_isolating(bool value) { ffi::set_use_isolating(m_bundle, value); }

        auto has_message(std::string_view id) -> bool {
            bool result = false;
            ffi::has_message(m_bundle, id.data(), &result);
            return result;
        }

        auto get_message(std::string_view id) -> FluencyMessage {
            ffi::FluencyMessage* msg = nullptr;
            ffi::get_message(m_bundle, id.data(), &msg);
            return FluencyMessage(msg);
        }

        auto format(const FluencyMessage& message, std::vector<std::string>& errors)
            -> std::string {
            char* output = nullptr;
            char** errors_c = nullptr;
            size_t error_length = 0;
            auto result = ffi::format_message(
                m_bundle, message.m_message, nullptr, &output, &errors_c, &error_length
            );
            if (result != ffi::FluencyResult::Ok) {
                throw std::invalid_argument("Message does not contain a value");
            }

            errors.clear();
            if (error_length != 0) {
                for (auto i = 0; i < error_length; ++i) { errors.emplace_back(errors_c[i]); }
                ffi::destroy_str_array(errors_c, error_length);
            }

            std::string output_str(output);
            ffi::destroy_string(output);

            return output_str;
        }

        auto format(
            const FluencyMessage& message, const FluencyArgs& args, std::vector<std::string>& errors
        ) -> std::string {
            char* output = nullptr;
            char** errors_c = nullptr;
            size_t error_length = 0;
            auto result = ffi::format_message(
                m_bundle, message.m_message, args.m_args, &output, &errors_c, &error_length
            );
            if (result != ffi::FluencyResult::Ok) {
                throw std::invalid_argument("Message does not contain a value");
            }

            errors.clear();
            if (error_length != 0) {
                for (auto i = 0; i < error_length; ++i) { errors.emplace_back(errors_c[i]); }
                ffi::destroy_str_array(errors_c, error_length);
            }

            std::string output_str(output);
            ffi::destroy_string(output);

            return output_str;
        }

        auto format(const FluencyAttribute& attribute, std::vector<std::string>& errors)
            -> std::string {
            char* output = nullptr;
            char** errors_c = nullptr;
            size_t error_length = 0;
            auto result = ffi::format_attribute(
                m_bundle, attribute.m_attribute, nullptr, &output, &errors_c, &error_length
            );
            if (result != ffi::FluencyResult::Ok) { throw std::invalid_argument("???"); }

            errors.clear();
            if (error_length != 0) {
                for (auto i = 0; i < error_length; ++i) { errors.emplace_back(errors_c[i]); }
                ffi::destroy_str_array(errors_c, error_length);
            }

            std::string output_str(output);
            ffi::destroy_string(output);

            return output_str;
        }

        auto format(
            const FluencyAttribute& attribute, const FluencyArgs& args,
            std::vector<std::string>& errors
        ) -> std::string {
            char* output = nullptr;
            char** errors_c = nullptr;
            size_t error_length = 0;
            auto result = ffi::format_attribute(
                m_bundle, attribute.m_attribute, args.m_args, &output, &errors_c, &error_length
            );
            if (result != ffi::FluencyResult::Ok) { throw std::invalid_argument("???"); }

            errors.clear();
            if (error_length != 0) {
                for (auto i = 0; i < error_length; ++i) { errors.emplace_back(errors_c[i]); }
                ffi::destroy_str_array(errors_c, error_length);
            }

            std::string output_str(output);
            ffi::destroy_string(output);

            return output_str;
        }
    };
} // namespace fluency