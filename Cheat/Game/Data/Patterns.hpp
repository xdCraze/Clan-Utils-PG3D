#pragma once

namespace Patterns
{
	// Class Patterns
	IL2CPP::FieldPattern ProfileClass = { "String", "Int32", "String", "Boolean", "Boolean", "Nullable`1", "Boolean", "String", "String", "Action" };

	// Method Patterns

	// Initializate
	void Init()
	{
		IL2CPP::AttachCurrentThread();
		IL2CPP::ClassMapping::AddImageToScan(IL2CPP::CurrentDomain()->OpenAssembly("Assembly-CSharp.dll"));
		IL2CPP::ClassMapping::AddQueue("ProfileClass", "", &ProfileClass);
		IL2CPP::ClassMapping::StartMapping();
	}
}