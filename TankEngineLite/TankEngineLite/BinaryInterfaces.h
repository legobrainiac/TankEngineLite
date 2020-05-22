#ifndef BINARY_WRITER_H
#define BINARY_WRITER_H

#include <iostream>
#include <fstream>
#include <ios>

class BinaryWriter
{
public:
	BinaryWriter(const std::string& fileName)
		: m_File(fileName, std::ios::out | std::ios::binary)
	{
	}

	~BinaryWriter()
	{
		m_File.close();
	}

	template<typename T>
	inline void Write(const T& val)
	{
		static_assert(std::is_pod<T>::value, "None POD value given for T");
		m_File.write((const char*)& val, sizeof(T));
	}

	template<>
	inline void Write<std::string>(const std::string& val)
	{
		size_t strSize = val.size();

		m_File.write((const char*)& strSize, sizeof(strSize));
		m_File.write((const char*) & *val.cbegin(), sizeof(char) * strSize);
	}

private:
	std::ofstream m_File;
};

class BinaryReader
{
public:
	BinaryReader(const std::string& fileName)
		: m_File(fileName, std::ios::in | std::ios::binary)
	{
	}

	~BinaryReader()
	{
		m_File.close();
	}

	inline void SetHead(int head)
	{
		m_File.seekg(head);
	}

	inline void MoveHead(int amnt)
	{
		SetHead((int)m_File.tellg() + amnt);
	}

	template<typename T>
	[[nodiscard]] inline T Read()
	{
		static_assert(std::is_pod<T>::value, "None POD value given for T");

		T obj;
		m_File.read((char*)& obj, sizeof(T));

		return obj;
	}

	template<>
	[[nodiscard]] inline std::string Read()
	{
		size_t strSize{};
		std::string str{};

		// Read string size and resize the receiving container
		m_File.read((char*)& strSize, sizeof(int));
		str.resize(strSize);

		// Read the memory in
		m_File.read((char*) & *str.cbegin(), sizeof(char) * strSize);

		return str;
	}

private:
	std::ifstream m_File;
};

#endif

