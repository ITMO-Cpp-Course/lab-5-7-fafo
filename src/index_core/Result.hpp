#pragma once

#include <expected>
#include <string>

enum class ErrorCode
{
    kSuccess = 0,
    kDuplicateDocumentId,
    kDocumentNotFound,
    kTransactionInactive,
    kEmptyWord,
    kInvalidArgument
};

template <typename T> using Result = std::expected<T, ErrorCode>;

using VoidResult = std::expected<void, ErrorCode>;