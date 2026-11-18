#include "text_util.hpp"

std::string_view Spliterator::Iterator::getCurrentToken() const
{
	size_t nextOffset = _stringView.find(_delimiter);
	if (nextOffset == std::string::npos) {
		return _stringView;
	}
	return std::string_view(_stringView.begin(), nextOffset);
}

std::string_view Spliterator::Iterator::operator*() const {
	std::string_view token = getCurrentToken();
	if (_trim) {
		size_t textBegin = token.find_first_not_of(WHITESPACE_CHARS);
		if (textBegin == std::string::npos) return "";
		size_t textEnd = token.find_last_not_of(WHITESPACE_CHARS);
		if (textEnd != std::string::npos) {
			return token.substr(textBegin, textEnd - textBegin + 1);
		} else {
			return "";
		}
	}
	return token;
}

Spliterator::Iterator& Spliterator::Iterator::operator++() {
	std::string_view currentToken = getCurrentToken();
	if (currentToken == _stringView) {
		if (currentToken.size() != 0) {
			_stringView = std::string_view(_stringView.cend(), 0);
		}
		if (!_atEnd) {
			_atEnd = true;
		}
	} else {
		_stringView = _stringView.substr(currentToken.size() + _delimiter.size());
	}
	return *this;
}