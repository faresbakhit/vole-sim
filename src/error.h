#pragma once

namespace vole {
namespace error {
enum class LoadProgramError {
	NOT_AN_ERROR,
	FILE_OPEN_FAILED,
	STREAM_READ_FAILED,
	TOO_MUCH_INSTRUCTIONS,
};
}
} // namespace vole
