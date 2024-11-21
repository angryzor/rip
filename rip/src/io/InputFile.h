#pragma once

template<typename T>
class InputFile {
public:
	virtual ~InputFile() = default;
	virtual T* getData() = 0;
};
