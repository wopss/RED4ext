#include "ScriptValidationError.hpp"

#include "App.hpp"

ValidationError ValidationError::FromString(const char* str)
{
    ValidationErrorType type = ValidationErrorType::Unknown;
    char name[64] = {0};
    char parent[64] = {0};

    if (sscanf_s(str, "Missing native class '%[^']'", name, (int)sizeof(name)) == 1)
    {
        type = ValidationErrorType::MissingClass;
    }
    else if (sscanf_s(str, "Missing native global function '%[^']'", name, (int)sizeof(name)) == 1)
    {
        type = ValidationErrorType::MissingGlobalFunction;
    }
    else if (sscanf_s(str, "Missing native function '%[^']' in native class '%[^']'", name, (int)sizeof(name), parent,
                      (int)sizeof(parent)) == 2)
    {
        type = ValidationErrorType::MissingMethod;
    }
    else if (sscanf_s(str, "Missing native property '%[^']' in native class '%[^']'", name, (int)sizeof(name), parent,
                      (int)sizeof(parent)) == 2)
    {
        type = ValidationErrorType::MissingProperty;
    }
    else if (sscanf_s(str, "Missing base class '%[^']' of native class '%[^']'", parent, (int)sizeof(parent), name,
                      (int)sizeof(name)) == 2)
    {
        type = ValidationErrorType::MissingBaseClass;
    }
    else if (sscanf_s(
                 str,
                 "Native class '%[^']' has declared base class '%[^']' that is different than current one '%*[^']'",
                 name, (int)sizeof(name), parent, (int)sizeof(parent)) == 2)
    {
        type = ValidationErrorType::BaseClassMismatch;
    }
    else if (sscanf_s(str, "Imported property '%[^.].%[^']' type '%*[^']' does not match with the native one '%*[^']'",
                      parent, (int)sizeof(parent), name, (int)sizeof(name)) == 2)
    {
        type = ValidationErrorType::PropertyTypeMismatch;
    }

    return {.type = type, .name = name, .parent = parent};
}

std::optional<SourceRef> ValidationError::GetSourceRef() const
{
    auto& sourceRepo = App::Get()->GetScriptCompilationSystem()->GetSourceRefRepository();

    try
    {
        switch (type)
        {
        case ValidationErrorType::MissingClass:
            return sourceRepo.GetClass(name);
        case ValidationErrorType::MissingGlobalFunction:
            return sourceRepo.GetFunction(name);
        case ValidationErrorType::MissingMethod:
            return sourceRepo.GetMethod(name, parent);
        case ValidationErrorType::MissingProperty:
            return sourceRepo.GetProperty(name, parent);
        case ValidationErrorType::MissingBaseClass:
            return sourceRepo.GetClass(name);
        case ValidationErrorType::BaseClassMismatch:
            return sourceRepo.GetClass(name);
        case ValidationErrorType::PropertyTypeMismatch:
            return sourceRepo.GetProperty(name, parent);
        default:
            return {};
        }
    }
    catch (std::out_of_range)
    {
        return {};
    }
}
