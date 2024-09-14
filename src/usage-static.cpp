/*! \file usage-static.cpp
    \brief Defines the functions for the static library.
    \author Christophe COUAILLET
*/

#include <array>
#include <iostream>
#include <cassert>
#include <sstream>

#include <str-utils-static.hpp>
#include <usage-static.hpp>

static std::string AType_toStr(Usage::Argument_Type arg)
{
    static const std::array<const std::string, 3> AType_Label{ "string", "boolean", "simple" };
    return AType_Label[(int)arg];
}

Usage::Argument::Argument(const std::string& name)
{
    m_name = name;
}

Usage::Argument::Argument(const Argument& argument)
{
    m_name = argument.m_name;
    m_required = argument.m_required;
    helpstring = argument.helpstring;
    for (auto val : argument.value)
        value.push_back(val);
}

Usage::Argument::Argument(const Argument* argument)
{
    m_name = argument->m_name;
    m_required = argument->m_required;
    helpstring = argument->helpstring;
    for (auto val : argument->value)
        value.push_back(val);
}

std::string Usage::Argument::name() const noexcept
{
    return m_name;
}

namespace Usage
{
    std::ostream& operator<<(std::ostream& os, const Argument& arg)
    {   // used to return the xml definition of the Argument
        return arg.print(os);
    }
}

std::ostream& Usage::Argument::print(std::ostream& os, const std::string& indent) const
{
    os << indent << "<name>" << m_name << "</name>" << std::endl;
    os << indent << "<helpstring>" << helpstring << "</helpstring>" << std::endl;
    os << indent << "<required>" << std::boolalpha << m_required << "</required>" << std::endl;
    return os;
}

Usage::Named_Arg::Named_Arg(const Named_Arg& argument) : Argument(argument)
{
    shortcut_char = argument.shortcut_char;
    m_type = argument.m_type;
    m_default_value = argument.m_default_value;
}

Usage::Named_Arg::Named_Arg(const Named_Arg* argument) : Argument(argument)
{
    shortcut_char = argument->shortcut_char;
    m_type = argument->m_type;
    m_default_value = argument->m_default_value;
}

std::ostream& Usage::Named_Arg::print(std::ostream& os, const std::string& indent) const
{
    os << indent << "<named>" << std::endl;
    Argument::print(os, indent + "\t");
    os << indent << "\t<shortcut_char>" << shortcut_char << "</shortcut_char>" << std::endl;
    os << indent << "\t<type>" << (int)m_type << "</type>" << std::endl;
    os << indent << "\t<default_value>" << m_default_value << "</default_value>" << std::endl;
    os << indent << "</named>" << std::endl;
    return os;
}

std::ostream& Usage::Unnamed_Arg::print(std::ostream& os, const std::string& indent) const
{
    os << indent << "<unnamed>" << std::endl;
    Argument::print(os, indent + "\t");
    os << indent << "\t<many>" << std::boolalpha << many << "</many>" << std::endl;
    os << indent << "</unnamed>" << std::endl;
    return os;
}

void Usage::Named_Arg::set_required(const bool required)
{
    assert((!required || required && m_default_value.empty()) && "An argument can't be required if it defines a default value.");
    m_required = required;
}

void Usage::Named_Arg::set_type(Argument_Type type)
{
    assert((type != Argument_Type::simple || type == Argument_Type::simple && m_default_value.empty()) && "Type simple can't be set for arguments with a default value.");
    m_type = type;
}

void Usage::Named_Arg::set_default_value(const std::string& default_value)
{
    assert((default_value.empty() || !default_value.empty() && !m_required) && "A default value can't be set for a required argument.");
    assert((default_value.empty() || !default_value.empty() && m_type != Argument_Type::simple) && "A default value can't be set for an argument of type simple.");
    m_default_value = default_value;
}

Usage::Usage::Usage(const std::string& prog_name)
{
    program_name = prog_name;
}

Usage::Usage::~Usage()
{
    for (auto arg : m_arguments)
        delete arg.second;      // args are dynamically created when added
}

void Usage::Usage::add_Argument(const Argument& argument)
{
    auto itr = m_arguments.find(argument.name());
    assert(itr == m_arguments.end() && "Argument already exists.");
    Argument* arg;
    if (argument.named())
        arg = new Named_Arg{ dynamic_cast<const Named_Arg*>(&argument) };
    else
        arg = new Unnamed_Arg{ dynamic_cast<const Unnamed_Arg*>(&argument) };
    m_arguments[argument.name()] = arg;
    m_argsorder.push_back(m_arguments[argument.name()]);
    m_syntax_valid = false;
}

void Usage::Usage::remove_Argument(const std::string& name)
{
    auto itr = m_arguments.find(name);
    assert(itr != m_arguments.end() && "Unknown argument name.");
    auto ord = std::find(m_argsorder.begin(), m_argsorder.end(), (*itr).second);
    if (ord != m_argsorder.end())
    {
        m_requirements.remove_all(*ord);
        m_conflicts.remove(*ord);
        m_argsorder.erase(ord);
    }
    Argument* arg = (*itr).second;
    m_arguments.erase(name);
    delete arg;         // was dynamically created when added
    m_syntax_valid = false;
}

void Usage::Usage::remove_all() noexcept
{
    // we must free memory for each dynamically created arg
    while (!m_arguments.empty())
        remove_Argument(m_arguments[0]->name());
    m_syntax_valid = false;
}

void Usage::Usage::clear() noexcept
{
    remove_all();
    program_name.clear();
    description.clear();
    usage.clear();
    m_syntax_valid = false;
}

Usage::Argument* Usage::Usage::get_Argument(const std::string& name)
{
    auto arg_itr = m_arguments.find(name);
    if (arg_itr != m_arguments.end())
        return (*arg_itr).second;
    return NULL;
}

std::vector<Usage::Argument*> Usage::Usage::get_Arguments()
{
    std::vector<Argument*> result{};
    for (auto arg : m_argsorder)
        result.push_back(arg);
    return result;
}

std::unordered_map<std::string, std::vector<std::string>> Usage::Usage::get_values() const
{
    std::unordered_map<std::string, std::vector<std::string>> result{};
    for (auto arg : m_argsorder)
        result[arg->name()] = arg->value;
    return result;
}

std::vector<std::string> Usage::Usage::get_values(const std::string& name) const
{
    auto itr = m_arguments.find(name);
    assert(itr != m_arguments.end() && "Unknown argument name.");
    return (*itr).second->value;
}

void Usage::Usage::add_requirement(const std::string& dependent, const std::string& requirement)
{
    assert((!dependent.empty() && !requirement.empty()) && "Requirements cannot be created without arguments name.");
    assert((dependent != requirement) && "An argument cannot require itself.");
    auto itr1 = m_arguments.find(dependent);
    auto itr2 = m_arguments.find(requirement);
    assert((itr1 != m_arguments.end() && itr2 != m_arguments.end()) && "Unknown argument name.");
    // define a requirement on a unrequired argument for a required argument has no sense - yes, it can be used to differentiate [a b] and [a [b]]
    // assert((!(*itr1).second->required() || (*itr1).second->required() == (*itr2).second->required()) && "A required argument can't depend on an unrequired argument.");
    // define a requirement has no sense for arguments in conflict
    assert(!(m_conflicts.in_conflict((*itr1).second, (*itr2).second)) && "A requirement can not be set for arguments in conflict.");
    // we must ensure the pair does not already exist
    assert(!m_requirements.exists((*itr1).second, (*itr2).second) && "Requirement is already defined.");
    m_requirements.add((*itr1).second, (*itr2).second);
    m_syntax_valid = false;
}

void Usage::Usage::remove_requirement(const std::string& dependent, const std::string& requirement)
{
    auto itr1 = m_arguments.find(dependent);
    auto itr2 = m_arguments.find(requirement);
    assert((itr1 != m_arguments.end() && itr2 != m_arguments.end()) && "Unknown argument name.");
    assert(m_requirements.exists((*itr1).second, (*itr2).second) && "Requirement does not exist.");
    m_requirements.remove((*itr1).second, (*itr2).second);
    m_syntax_valid = false;
}

void Usage::Usage::remove_requirements(const std::string& argument)
{
    auto itr = m_arguments.find(argument);
    assert(itr != m_arguments.end() && "Unknown argument name.");
    assert(m_requirements.has_requirements((*itr).second) && "No requirement exists for this argument.");
    m_requirements.remove_requirement((*itr).second);
    m_syntax_valid = false;
}

void Usage::Usage::clear_requirements() noexcept
{
    m_requirements.clear();
    m_syntax_valid = false;
}

bool Usage::Usage::requirement_exists(const std::string& dependent, const std::string& requirement) const
{
    auto const itr1 = m_arguments.find(dependent);
    auto const itr2 = m_arguments.find(requirement);
    assert((itr1 != m_arguments.end() && itr2 != m_arguments.end()) && "Unknown argument name.");
    return m_requirements.exists((*itr1).second, (*itr2).second);
}

bool Usage::Usage::has_requirements(const std::string& dependent) const
{
    auto itr = m_arguments.find(dependent);
    assert((itr != m_arguments.end()) && "Unknown argument name.");
    return m_requirements.has_requirements((*itr).second);
}

bool Usage::Usage::has_dependents(const std::string& requirement) const
{
    auto itr = m_arguments.find(requirement);
    assert((itr != m_arguments.end()) && "Unknown argument name.");
    return m_requirements.has_dependents((*itr).second);
}

Usage::Argument* Usage::Usage::get_requirement(const std::string& dependent, const std::string& requirement)
{
    auto itr1 = m_arguments.find(dependent);
    auto itr2 = m_arguments.find(requirement);
    assert((itr1 != m_arguments.end() && itr2 != m_arguments.end()) && "Unknown argument name.");
    assert(m_requirements.exists((*itr1).second, (*itr2).second) && "Requirement does not exist.");
    return (*itr2).second;
}

std::vector<std::string> Usage::Usage::get_requirements(const std::string& argument) const
{
    auto itr = m_arguments.find(argument);
    assert(itr != m_arguments.end() && "Unknown argument name.");
    std::vector<std::string> result{};
    std::vector<const Argument*> requirements{ m_requirements.requirements((*itr).second) };
    for (auto req : requirements)
        result.push_back(req->name());
    return result;
}

std::vector<std::string> Usage::Usage::get_dependents(const std::string& argument) const
{
    auto itr = m_arguments.find(argument);
    assert(itr != m_arguments.end() && "Unknown argument name.");
    std::vector<std::string> result{};
    std::vector<const Argument*> dependents{ m_requirements.dependents((*itr).second) };
    for (auto dep : dependents)
        result.push_back(dep->name());
    return result;
}

std::unordered_multimap<std::string, std::string> Usage::Usage::get_requirements() const
{
    std::unordered_multimap<std::string, std::string> result{};
    auto requirements = m_requirements.get();
    auto itr = requirements.begin();
    while (itr != requirements.end())
    {
        result.insert({ (*itr).first->name(), (*itr).second->name() });
        ++itr;
    }
    return result;
}

void Usage::Usage::set_requirements(const std::unordered_multimap<std::string, std::string>& requirements)
{
    auto itr = requirements.begin();
    while (itr != requirements.end())
    {
        add_requirement((*itr).first, (*itr).second);
        ++itr;
    }
    m_syntax_valid = false;
}

void Usage::Usage::add_conflict(const std::string& arg1, const std::string& arg2)
{
    assert((!arg1.empty() && !arg2.empty()) && "Conflicts cannot be created without arguments name.");
    assert((arg1 != arg2) && "An argument cannot be in conflict with itself.");
    auto itr1 = m_arguments.find(arg1);
    auto itr2 = m_arguments.find(arg2);
    assert((itr1 != m_arguments.end() && itr2 != m_arguments.end()) && "Unknown argument name.");
    assert(((*itr1).second->required() == (*itr2).second->required()) && "All arguments in conflict must be either required or unrequired.");
    // set a conflict for arguments linked by a requirement has no sense
    assert(!(m_requirements.exists((*itr1).second, (*itr2).second, true) || m_requirements.exists((*itr2).second, (*itr1).second, true)) && "Dependent arguments cannot be in conflict.");
    // we must ensure the pair does not already exist for the 2 directions (name, conflict) and (conflict, name)
    assert(!(m_conflicts.in_conflict((*itr1).second, (*itr2).second)) && "Conflict already exists.");
    m_conflicts.add((*itr1).second, (*itr2).second);
    m_syntax_valid = false;
}

void Usage::Usage::remove_conflict(const std::string& arg1, const std::string& arg2)
{
    auto itr1 = m_arguments.find(arg1);
    auto itr2 = m_arguments.find(arg2);
    assert((itr1 != m_arguments.end() && itr2 != m_arguments.end()) && "Unknown argument name.");
    // conflict definition is searched for the 2 directions (name, conflict) and (conflict, name)
    assert(m_conflicts.in_conflict((*itr1).second, (*itr2).second) && "Conflict does not exist.");
    m_conflicts.remove((*itr1).second, (*itr2).second);
    m_syntax_valid = false;
}

void Usage::Usage::remove_conflicts(const std::string& argument)
{
    auto itr = m_arguments.find(argument);
    assert(itr != m_arguments.end() && "Unknown argument name.");
    // search must be done for 2 directions (name, x) and (x, name)
    assert(m_conflicts.in_conflict((*itr).second) && "No conflict exists for this argument.");
    m_conflicts.remove((*itr).second);
    m_syntax_valid = false;
}

void Usage::Usage::clear_conflicts() noexcept
{
    m_conflicts.clear();
    m_syntax_valid = false;
}

bool Usage::Usage::in_conflict(const std::string& argument) const
{
    auto itr = m_arguments.find(argument);
    assert(itr != m_arguments.end() && "Unknown argument name.");
    return m_conflicts.in_conflict((*itr).second);
}

bool Usage::Usage::in_conflict(const std::string& arg1, const std::string& arg2) const
{
    auto itr1 = m_arguments.find(arg1);
    auto itr2 = m_arguments.find(arg2);
    assert((itr1 != m_arguments.end() && itr2 != m_arguments.end()) && "Unknown argument name.");
    return m_conflicts.in_conflict((*itr1).second, (*itr2).second);
}

Usage::Argument* Usage::Usage::get_conflict(const std::string& arg1, const std::string& arg2)
{
    auto itr1 = m_arguments.find(arg1);
    auto itr2 = m_arguments.find(arg2);
    assert((itr1 != m_arguments.end() && itr2 != m_arguments.end()) && "Unknown argument name.");
    assert(m_conflicts.in_conflict((*itr1).second, (*itr2).second) && "These arguments are not in conflict.");
    return (*itr2).second;
}

std::vector<std::string> Usage::Usage::get_conflicts(const std::string& argument) const
{
    auto itr = m_arguments.find(argument);
    assert(itr != m_arguments.end() && "Unknown argument name.");
    std::vector<std::string> result{};
    auto conflicts = m_conflicts.conflicts((*itr).second);
    for (auto con : conflicts)
        result.push_back(con->name());
    return result;
}

std::unordered_multimap<std::string, std::string> Usage::Usage::get_conflicts() const
{
    auto conflicts = m_conflicts.get();
    std::unordered_multimap<std::string, std::string> result{};
    auto itr = conflicts.begin();
    while (itr != conflicts.end())
    {
        result.insert({ (*itr).first->name(), (*itr).second->name() });
        ++itr;
    }
    return result;
}

void Usage::Usage::set_conflicts(const std::unordered_multimap<std::string, std::string>& conflicts)
{
    auto itr = conflicts.begin();
    while (itr != conflicts.end())
    {
        add_conflict((*itr).first, (*itr).second);
        ++itr;
    }
    m_syntax_valid = false;
}

void Usage::Usage::load_from_file(const std::string& fname)
{
    m_syntax_valid = true;
}

void Usage::Usage::save_to_file(const std::string& fname) const
{}

void Usage::Usage::set_syntax(const std::string& syntax)
{
    m_syntax_string = syntax;
    m_syntax_valid = true;
}

bool Usage::Usage::m_check_unnamed(const std::string& value, std::vector<bool>& set_args, bool& many, size_t& unnamed)
{
    bool found{ false };

    if (many)
        m_argsorder[unnamed]->value.push_back(value);
    else
    {
        for (size_t i = 0; i < m_argsorder.size(); i++)
        {
            if (!m_argsorder[i]->named() && !set_args[i])
            {
                m_argsorder[i]->value.push_back(value);
                set_args[i] = true;
                many = dynamic_cast<Unnamed_Arg*>(m_argsorder[i])->many;
                unnamed = i;
                found = true;
                break;
            }
        }
    }
    return found;
}

std::string Usage::Usage::m_check_type(const std::string& p, Argument_Type type_p, const std::string& value, std::vector<bool>& set_args)
{
    static const std::string switch_str{ switch_char };
    static const std::string TYPE_MISMATCH{ "Argument '%s' passed as '%s' while expected type is '%s' - see %s " + switch_str + help_arg + " for help." };
    static const std::string UNKNOW_ARGUMENT{ "Unknown argument '" + switch_str + "%s' - see %s " + switch_str + help_arg + " for help." };

    bool found{ false };
    for (size_t i = 0; i < m_argsorder.size(); i++)
    {
        std::string name2{};
        if (m_argsorder[i]->named() && !set_args[i])
        {
            name2 = dynamic_cast<Named_Arg*>(m_argsorder[i])->shortcut_char;
            if (p == m_argsorder[i]->name() || p == name2)
            {
                Argument_Type type_a = dynamic_cast<Named_Arg*>(m_argsorder[i])->type();
                if (type_p != type_a)
                    return str_utils::get_message(TYPE_MISMATCH.c_str(), m_argsorder[i]->name().c_str(),
                        AType_toStr(type_p).c_str(), AType_toStr(type_a).c_str(), program_name.c_str());
                m_argsorder[i]->value.push_back(value);
                set_args[i] = true;
                found = true;
                break;
            }
        }
    }
    if (!found)
        return str_utils::get_message(UNKNOW_ARGUMENT.c_str(), p.c_str(), program_name.c_str());
    // All is fine
    return "";
}


std::string Usage::Usage::m_parser(int argc, char* argv[], std::vector<bool>& set_args)
{
    static const std::string switch_str{ switch_char };
    static const std::string SYNTAX_ERROR{ "Error found in command line argument number %i: '%s' - see %s " + switch_str + help_arg + " for help." };

    bool many{ false };
    size_t unnamed{ 0 };
    for (size_t i = 1; i < (size_t)argc; i++)
    {
        std::string p = argv[i];
        if (p.empty())
            continue;
        bool named{ p[0] == switch_char };
        if (named)
            p.erase(0, 1);
        if (p.empty())
            return str_utils::get_message(SYNTAX_ERROR.c_str(), i, argv[i], program_name.c_str(), switch_char, help_arg);
        if (p == help_arg)
            // Help requested
            return "?";
        auto quote = p.find('\"');
        bool quoted{ quote != std::string::npos };
        std::string value{};
        if (!named)
        {
            value = p;
            if (quoted)
            {
                // TODO format value inside quotes
            }
            if (!m_check_unnamed(value, set_args, many, unnamed))
                return str_utils::get_message(SYNTAX_ERROR.c_str(), i, argv[i], program_name.c_str());
            continue;
        }
        many = false;
        if (quoted)
        {
            value = p.substr(quote + 1, p.length() - quote - 1);
            p.erase(quote, p.length() - 1);
            // TODO format value inside quotes
        }
        if (p.empty())
            return str_utils::get_message(SYNTAX_ERROR.c_str(), i, argv[i], program_name.c_str());
        Argument_Type type_p{ Argument_Type::simple };
        auto colon = p.find(':');
        if (colon != std::string::npos)
        {
            if (colon < p.size() - 1)
                value = p.substr(colon + 1, p.size() - colon) + value;
            type_p = Argument_Type::string;
            p.erase(colon, p.length() - colon);
        }
        else
        {
            auto sgn = p[p.length() - 1];
            if (sgn == '+' || sgn == '-')
            {
                type_p = Argument_Type::boolean;
                p.erase(p.length() - 1);
                if (!value.empty())
                    return str_utils::get_message(SYNTAX_ERROR.c_str(), i, argv[i], program_name.c_str());
                value = "false";
                if (sgn == '+')
                    value = "true";
            }
            else
            {
                // simple argument
                if (!value.empty())
                    return str_utils::get_message(SYNTAX_ERROR.c_str(), i, argv[i], program_name.c_str());
                value = "true";
            }
        }
        if (p.empty())
            return str_utils::get_message(SYNTAX_ERROR.c_str(), i, argv[i], program_name.c_str());
        auto ret = m_check_type(p, type_p, value, set_args);
        if (ret != "")
            return ret;
    }
    // All is fine and values are affected to arguments
    return "";

}

void Usage::Usage::m_check_requirements(size_t arg_index, std::vector<bool>& set_args)
{
    if (!set_args[arg_index] && m_argsorder[arg_index]->named())
    {
        auto dval = dynamic_cast<Named_Arg*>(m_argsorder[arg_index])->default_value();
        if (!dval.empty())
        {
            // default value must be applied only if required args are effectively used
            auto reqs = m_requirements.requirements(m_argsorder[arg_index]);
            bool req_defined{ false };
            if (reqs.empty())
                req_defined = true;     // always apply default value for non-dependent args
            for (auto req : reqs)
            {
                auto itr = std::find(m_argsorder.begin(), m_argsorder.end(), req);
                if (itr != m_argsorder.end())
                {
                    auto j = std::distance(m_argsorder.begin(), itr);
                    if (set_args[j])
                    {
                        req_defined = true;
                        break;
                    }
                }
            }
            if (req_defined)
            {
                m_argsorder[arg_index]->value.push_back(dval);
                set_args[arg_index] = true;
            }
        }
    }
}

size_t Usage::Usage::m_check_argument(std::vector<bool>& set_args)
{
    for (size_t i = 0; i < m_argsorder.size(); i++)
    {
        if (!set_args[i] && m_argsorder[i]->required())
        {
            // inspect args in direct conflict with the current one
            auto cons = m_conflicts.conflicts(m_argsorder[i]);
            bool con_defined{ false };
            for (auto con : cons)
            {
                auto itr = std::find(m_argsorder.begin(), m_argsorder.end(), con);
                if (itr != m_argsorder.end())
                {
                    auto j = std::distance(m_argsorder.begin(), itr);
                    if (set_args[j])
                    {
                        con_defined = true;
                        break;
                    }
                }
            }
            if (!con_defined)
                return i;
        }
        m_check_requirements(i, set_args);
    }
    // All is fine
    return -1;
}

std::string Usage::Usage::m_check_dependencies(std::vector<bool>& set_args)
{
    static const std::string switch_str{ switch_char };
    static const std::string REQUIRED_ARGUMENT{ "Missing required argument '%s' - see %s " + switch_str + help_arg + " for help." };
    static const std::string CONFLICT{ "Arguments '%s' and '%s' can't be used together - see %s " + switch_str + help_arg + " for help." };

    auto ret = m_check_argument(set_args);
    if (ret != -1)
        return str_utils::get_message(REQUIRED_ARGUMENT.c_str(), m_argsorder[ret]->name().c_str(), program_name.c_str());
    for (size_t i = 0; i < m_argsorder.size(); i++)
    {
        if (set_args[i])
        {
            for (size_t j = 0; j < m_argsorder.size(); j++)
            {
                if (i != j)
                {
                    if (set_args[j] && m_conflicts.in_conflict(m_argsorder[i], m_argsorder[j]))
                        return str_utils::get_message(CONFLICT.c_str(), m_argsorder[i]->name().c_str(), m_argsorder[j]->name().c_str(), program_name.c_str());
                    if (!set_args[j] && m_requirements.exists(m_argsorder[i], m_argsorder[j], true))
                        return str_utils::get_message(REQUIRED_ARGUMENT.c_str(), m_argsorder[j]->name().c_str(), program_name.c_str());
                }
            }
        }
    }
    // All is fine and values are affected to arguments
    return "";
}

std::string Usage::Usage::set_parameters(int argc, char* argv[])
{
    if (argc == 0)
        return "No argument to evaluate.";
    std::vector<bool> set_args(m_argsorder.size(), false);
    auto ret = m_parser(argc, argv, set_args);
    if (ret != "")
        return ret;
    ret = m_check_dependencies(set_args);
    return ret;
}

/* void Usage::Usage::create_syntax()
{
    m_syntax_string = program_name + " ";
    auto reqs = m_requirements.get();
    std::vector<std::pair<size_t, size_t>> requirements{};                     // first is the requirement and second the dependent
    std::vector<std::pair<size_t, size_t>> counts(m_argsorder.size(), { 0, 0 });  // first is the count as requirement and second the count as dependent
    for (auto req : reqs)
    {
        auto dep_itr = std::find(m_argsorder.begin(), m_argsorder.end(), req.first);
        auto req_itr = std::find(m_argsorder.begin(), m_argsorder.end(), req.second);
        assert((req_itr != m_argsorder.end() && dep_itr != m_argsorder.end()) && "Unexpected issue in function create_syntax.");
        auto dep_idx = std::distance(m_argsorder.begin(), dep_itr);
        auto req_idx = std::distance(m_argsorder.begin(), req_itr);
        requirements.push_back({ req_idx, dep_idx });
        counts[req_idx].first++;
        counts[dep_idx].second++;
    }
    for (size_t i = 1; i < m_argsorder.size(); i++)
        // adds a requirement with preceding arg for orphan unnamed args
        if (!m_argsorder[i]->named() && !m_requirements.has_requirements(m_argsorder[i]))
        {
            requirements.push_back({ i - 1, i });
            counts[i - 1].first++;
            counts[i].second++;
        }
    std::vector<std::vector<size_t>> conflicts{};
    std::vector<size_t> con_idx(m_argsorder.size(), -1);
    for (size_t i = 0; i < m_argsorder.size(); i++)
    {
        if (con_idx[i] == -1)       // each arg can be in only one group of conflict
        {
            auto cons = m_conflicts.all_conflicts(m_argsorder[i]);
            std::vector<size_t> row{};
            for (auto con : cons)
            {
                auto arg_itr = std::find(m_argsorder.begin(), m_argsorder.end(), con);
                auto arg_idx = std::distance(m_argsorder.begin(), arg_itr);
                row.push_back(arg_idx);
                con_idx[arg_idx] = conflicts.size();
            }
            conflicts.push_back(row);
        }
    }
    auto group_base = counts.size();                       // groups id starts at this index in table counts
    std::vector<std::vector<size_t>> groups{};
    bool transforms{ true };
    while (transforms)
    {
        transforms = false;
        auto req_itr = requirements.begin();
        while (req_itr != requirements.end())
        {
            // links orphan dependents to their requirement
            if (counts[(*req_itr).second].second == 1 && counts[(*req_itr).second].first == 0
                && (counts[(*req_itr).first].first != 0 || counts[(*req_itr).first].second != 0))
            {
                std::vector<size_t> row{ (*req_itr).first, (*req_itr).second };
                auto group_id = group_base + groups.size();
                groups.push_back(row);
                counts.push_back({ 0, 0 });
                auto con1 = con_idx[row[0]];
                auto con2 = con_idx[row[1]];
                assert((con1 == con2 || con1 == -1 || con2 == -1) && "Sorry I don't know how to handle requirements involved in distinct conflicts.");
                if (con1 != -1)
                    con_idx.push_back(con1);
                else
                    con_idx.push_back(con2);
                for (auto req : requirements)
                {
                    if (req.first == row[0])
                    {
                        counts[row[0]].first--;
                        req.first = group_id;
                        counts[group_id].first++;
                    }
                    else if (req.second == row[0])
                    {
                        counts[row[0]].second--;
                        req.second = group_id;
                        counts[group_id].second++;
                    }
                }
                req_itr = requirements.erase(req_itr);
                transforms = true;
            }
            else
                ++req_itr;
        }
        req_itr = requirements.begin();
        while (req_itr != requirements.end())
        {
            // links orphan requirements to their dependent
            if (counts[(*req_itr).first].first == 1 && counts[(*req_itr).first].second == 0
                && counts[(*req_itr).second].first != 0 && counts[(*req_itr).second].second == 1)
            {
                std::vector<size_t> row{ (*req_itr).first, (*req_itr).second };
                auto group_id = group_base + groups.size();
                groups.push_back(row);
                counts.push_back({ 0, 0 });
                counts[row[0]].first--;
                counts[row[1]].second--;
                auto con1 = con_idx[row[0]];
                auto con2 = con_idx[row[1]];
                assert((con1 == con2 || con1 == -1 || con2 == -1) && "Sorry I don't know how to handle requirements involved in distinct conflicts.");
                if (con1 != -1)
                    con_idx.push_back(con1);
                else
                    con_idx.push_back(con2);
                for (auto req : requirements)
                    if (req.first == row[1])
                    {
                        counts[row[1]].first--;
                        req.first = group_id;
                        counts[group_id].first++;
                    }
                req_itr = requirements.erase(req_itr);
                transforms = true;
            }
        }
    }
    size_t start{ 0 };
    if (m_requirements.has_requirements(m_argsorder[0]))
    {
        // search for starting argument
        auto req_init = m_requirements.all_requirements(m_argsorder[0]);
        size_t max_size{ 0 };
        auto req_itr = req_init.begin();
        auto req_found = req_init.end();
        while (req_itr != req_init.end())
        {
            // search for the longest chain
            if ((*req_itr).size() > max_size)
            {
                max_size = (*req_itr).size();
                req_found = req_itr;
            }
            ++req_itr;
        }
        auto arg_itr = std::find(m_argsorder.begin(), m_argsorder.end(), (*req_found).back());
        start = std::distance(m_argsorder.begin(), arg_itr);
    }
    // TODO
    m_syntax_valid = true;
} */

namespace Usage
{
    std::ostream& operator<<(std::ostream& os, Usage& us)
    {
        if (!us.syntax_is_valid())
        {
            // TODO review with create_syntax
            // us.create_syntax();
        }
        os << us.description << std::endl << std::endl;
        os << "Syntax:" << std::endl;
        os << "    " << us.m_syntax_string << std::endl << std::endl;
        // argsorder is used to list the elements in the same order they were added
        // do a first pass to determine the max length
        size_t max_length{ 0 };
        auto itr = us.m_argsorder.begin();
        while (itr != us.m_argsorder.end())
        {
            auto lgth = (*itr)->name().length();
            if ((*itr)->named() && dynamic_cast<Named_Arg*>(*itr)->shortcut_char != ' ')
                lgth += 3;
            if (lgth > max_length)
                max_length = lgth;
            ++itr;
        }
        std::string filler{ "" };
        filler.append(max_length, ' ');
        itr = us.m_argsorder.begin();
        while (itr != us.m_argsorder.end())
        {
            os << "    " << (*itr)->name();
            auto lgth = (*itr)->name().length();
            if ((*itr)->named() && dynamic_cast<Named_Arg*>(*itr)->shortcut_char != ' ')
            {
                os << ", " << dynamic_cast<Named_Arg*>(*itr)->shortcut_char;
                lgth += 3;
            }
            if (filler.length() > lgth)
            {
                std::string fill2{ "" };
                fill2.append(max_length - lgth, ' ');
                os << fill2;
            }
            // display the helpstring with indent
            std::stringstream ostr;
            ostr.str((*itr)->helpstring);
            std::array<char, 255> buf;
            bool indent{ false };
            while (ostr.getline(&buf[0], 255, '\n'))
            {
                if (indent)
                    os << "    " << filler;
                else
                    indent = true;
                os << "    " << &buf[0] << std::endl;
            }
            if ((*itr)->named())
            {
                auto dval = dynamic_cast<Named_Arg*>(*itr)->default_value();
                if (!dval.empty())
                {
                    os << "    " << filler << "    ";
                    if (dval == "\t")
                        os << "'Tab'";
                    else if (dval == " ")
                        os << "'Space'";
                    else
                        os << "'" << dval << "'";
                    os << " by default." << std::endl;
                }
            }
            ++itr;
        }
        os << std::endl;
        os << us.usage << std::endl;
        return os;
    }
}
