#pragma once

/*! \file usage.hpp
*	\brief Implements the classes Argument_Type, Argument, Named_Arg, Unnamed_Arg and Usage.
*   \author Christophe COUAILLET
*/

#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <requirements.hpp>
#include <conflicts.hpp>

/*! \brief Defines the types of Named_Arg (named argument) objects:
*   \li string: passed as Argument:value,
*   \li boolean: passed as Argument+ or Argument-,
*   \li simple: passed as Argument without additional value.
*/
enum class Argument_Type {
    string = 0,         // passed as Argument:value
    boolean = 1,        // passed as Argument+ or Argument-
    simple = 2          // passed as Argument without additional value
};

/*! \brief An abstract class that defines the bases of classes Named_Arg (named argument) and Unnamed_Arg (unnamed argument). */
class Argument
{
public:
    /*! \brief Returns true for Named_Arg objects, false for Unnamed_Arg objects. */
    virtual bool named() const noexcept = 0;
    /*! \brief Returns the name of the argument that is set at instantiation. */
    std::string name() const noexcept;
    /*! \brief Sets or gets the help string displayed in the usage output. */
    std::string helpstring{};
    /*! \brief Returns true if the argument is obligatory.
    *   \sa Argument::set_required()
    */
    bool required() const noexcept { return m_required; }
    /*! \brief list of values passed for the argument. Unnamed_Arg objects can contain many values. */
    std::vector<std::string> value{};

    Argument() = delete;
    /*! \brief Default constructor that sets the name of the argument. */
    Argument(const std::string& name);
    /*! \brief Copy constructor by reference. */
    Argument(const Argument& argument);
    /*! \brief Copy constructor by pointer. */
    Argument(const Argument* argument);

    /*! \brief Sets if the argument is obligatory or not. */
    virtual void set_required(const bool required) = 0;

    /*! \brief This override returns in the output stream os the xml definition of the argument. */
    friend std::ostream& operator<<(std::ostream& os, const Argument& argument);
    // used to return the xml definition of the Argument

protected:
    /*! \brief Sets or gets the name of the argument. */
    std::string m_name{};
    /*! \brief Sets or gets the obligatory status of the argument, false by default. */
    bool m_required{ false };

    /*! \brief Prints in the output stream the help string of the argument.
        \param indent Optional string inserted before the argument help.
    */
    virtual std::ostream& print(std::ostream& os, const std::string& indent = "") const;
};

/*! \brief Class for named arguments.

    A named argument is an argument for which the value is preceded with a name.
    In the current implementation of Usage class, a named argument always starts with '/' under Windows or '-' under Linux (switch char).
    A shortcut of one character length can be set for named arguments.
*/
class Named_Arg : public Argument
{
    // An argument that is named. Named args can be used in any order.
    // They are always preceded by /
public:

    virtual bool named() const noexcept override { return true; }
    /*! \brief Sets or gets the shortcut character. */
    char shortcut_char{ ' ' };
    /*! \brief Returns the type of the argument. */
    Argument_Type type() noexcept { return m_type; }
    /*! \brief Returns the value applied when the argument is not used. */
    std::string default_value() noexcept { return m_default_value; }

    Named_Arg() = delete;
    /*! \brief Default constructor that sets the name of the argument. */
    Named_Arg(const std::string& name) : Argument(name) {};
    /*! \brief Copy constructor by reference. */
    Named_Arg(const Named_Arg& argument);
    /*! \brief Copy constructor by pointer. */
    Named_Arg(const Named_Arg* argument);

    virtual void set_required(const bool required) override;
    /*! \brief Sets the type of the named argument. */
    void set_type(Argument_Type type);
    /*! \brief Sets the default value of the named argument. */
    void set_default_value(const std::string& default_value);

protected:
    /*! \brief Sets or gets the type of the argument. The argument type is simple by default. */
    Argument_Type m_type{ Argument_Type::simple };
    /*! \brief Sets or gets the default value of the argument. */
    std::string m_default_value{};

    virtual std::ostream& print(std::ostream& os, const std::string& indent = "") const override;
};

/*! \brief Class for unnamed arguments.

    Values of unnamed arguments are passed directly through the command line.
    An unnamed argument still has a name. It is used to handle rules and display help.
    Instead of named arguments, this name has not to be passed through the command line.
    A single unnamed argument can accept many following values (i.e. a list of files).
*/
class Unnamed_Arg : public Argument
{
    // An argument that is not named. Unnamed args must be passed in the expected order
public:
    virtual bool named() const noexcept override { return false; }
    /*! \brief Sets or gets the capability of the argument to accept many values. */
    bool many{ false };

    Unnamed_Arg() = delete;
    /*! \brief Default constructor that sets the name of the argument. */
    Unnamed_Arg(const std::string& name) : Argument(name) {};
    /*! \brief Copy constructor by reference. */
    Unnamed_Arg(const Unnamed_Arg& argument) : Argument(argument) { many = argument.many; }
    /*! \brief Copy constructor by pointer. */
    Unnamed_Arg(const Unnamed_Arg* argument) : Argument(argument) { many = argument->many; }

    virtual void set_required(const bool required) override { Argument::m_required = required; }

protected:
    virtual std::ostream& print(std::ostream& os, const std::string& indent = "") const override;
};

/*! \brief The class Usage handles the list of arguments and their rules.

    It is used to set the named or unnamed arguments the program expect, if they are required or not, their default and assigned values and so on.
    It allows to check if the arguments passed on the command line are compliant with the rules set and can display a complete help.
*/
class Usage
{
private:
    // table of arguments
    std::unordered_map<std::string, Argument*> m_arguments{};

    // this non sorted container is used to keep the order in which arguments are defined, mainly for unnamed arguments
    std::vector<Argument*> m_argsorder{};

    // arguments that requires use of other arguments
    Requirements<const Argument*> m_requirements{ false };

    // arguments delimited by | ; conflicting arguments must be either all required either all optional
    // if they are all required they are delimited by () ; if they are all optional they are delimited by []
    Conflicts<const Argument*> m_conflicts{ true };         // with cascading

    // command line syntax
    std::string m_syntax_string{};

    // build the command line syntax ; build_syntax is called each time the operator << is used
    //void create_syntax();               // TODO
    bool m_syntax_valid{ false };

public:

    /*! \brief Gets the switch char used to start a named arg. */
#ifdef _WIN32
    const char switch_char{ '/' };
#elif __unix__
    const char switch_char{ '-' };
#endif
    /*! \brief Gets the help argument. */
#ifdef _WIN32
    const std::string help_arg{ "?" };
#elif __unix__
    const std::string help_arg{ "h" };
#endif

    /*! \brief Sets or gets the name of the program. */
    std::string program_name{};
    /*! \brief Sets or gets the brief description of the program that is displayed before the syntax and arguments explanation. */
    std::string description{};
    /*! \brief Sets or gets the help text that is displayed after the syntax and arguments explanation, including examples. */
    std::string usage{};

    Usage() = delete;
    /*! \brief Default constructor that sets the program name.
        \note The absolute path of the program is present in the first argument argv[0] passed to main function.
    */
    Usage(const std::string& prog_name);    // program name is in argv[0] passed to main()
    /*! \brief Destructor. */
    ~Usage();
    /*! \brief Adds a named or unnamed argument to the list of arguments.
    
        An assertion occurs if an argument with the same name already exists.
    */
    void add_Argument(const Argument& argument);
    /*! \brief Removes the argument of the list by its name.
    
        An assertion occurs if the argument name is unknown.
    */
    void remove_Argument(const std::string& name);
    /*! \brief Removes all arguments from the list. */
    void remove_all() noexcept;
    /*! \brief Removes all arguments from the list and clears the description, syntax and usage help strings. */
    void clear() noexcept;
    /*! \brief Returns a pointer to the argument or NULL if the given name does not exist. */
    Argument* get_Argument(const std::string& name);
    /*! \brief Returns a list of pointers to arguments. */
    std::vector<Argument*> get_Arguments();
    /*! \brief Returns the whole list of pairs of argument names and values. */
    std::unordered_map<std::string, std::vector<std::string>> get_values() const;       // return couples name, values - to use after call of set_parameters
    /*! \brief Returns the list of values passed through the command line for the requested argument name.
    
        An assertion occurs if the argument name is unknown.
    */
    std::vector<std::string> get_values(const std::string& name) const;                 // return values for a single argument

    /*! \brief Sets a required argument for a dependent argument.
    
        An assertion occurs if:
        \li one of the two argument name is unknown,
        \li the two arguments are same,
        \li a conflict has yet been set between these arguments,
        \li this requirement has been set yet.
        \sa Requirements< T > without reflexivity.
    */
    void add_requirement(const std::string& dependent, const std::string& requirement);
    /*! \brief Removes the requirement between the two arguments.
    
        An assertion occurs if one of the two argument name is unknown or if the requirement does not exist.
    */
    void remove_requirement(const std::string& dependent, const std::string& requirement);
    /*! \brief Removes all the requirements for the given argument.
    * 
    *   An assertion occurs if the argument name is unknown or if no requirement is found.
    */
    void remove_requirements(const std::string& dependent);
    /*! \brief Clears all existing requirements. */
    void clear_requirements() noexcept;
    /*! \brief Returns true if the argument dependent requires the presence of the requirement argument.
    * 
    *   An assertion occurs if one of the argument name is unknown.
    */
    bool requirement_exists(const std::string& dependent, const std::string& requirement) const;
    /*! \brief Returns true if requirements exist for the given argument.
    * 
    *   An assertion occurs if the argument name is unknown.
    */
    bool has_requirements(const std::string& dependent) const;
    /*! \brief Returns true if dependents exist for the given argument.
    *
    *   An assertion occurs if the argument name is unknown.
    */
    bool has_dependents(const std::string& requirement) const;
    /*! \brief Returns a pointer to the given argument that is required by the argument dependent.
    * 
    *   An assertion occurs if one of the argument name is unknown or if the requirement does not exist.
    */
    Argument* get_requirement(const std::string& dependent, const std::string& requirement);
    /*! \brief Returns the list of argument names that are required by the given argument.
    * 
    *   An assertion occurs if the argument name is unknown.
    */
    std::vector<std::string> get_requirements(const std::string& dependent) const;
    /*! \brief Returns the list of argument names that depends on the given argument.
    * 
    *   An assertion occurs if the argument name is unknown.
    */
    std::vector<std::string> get_dependents(const std::string& requirement) const;
    /*! \brief Returns the list of pairs of dependencies by argument name (dependent name, requirement name) */
    std::unordered_multimap<std::string, std::string> get_requirements() const;
    /*! \brief Sets the requirements between arguments from the given list of name pairs.
    * 
    *   An assertion occurs if rules are broken.
    *   \sa Usage::add_requirement()
    */
    void set_requirements(const std::unordered_multimap<std::string, std::string>& requirements);

    /*! \brief Adds a conflict between the given arguments.
    *
        Arguments in conflict are delimited by '|' in the command syntax. They cannot be used together.
    *   An assertion occurs if:
    *   \li at least one of the argument name is unknown,
    *   \li arguments are same,
    *   \li the required status of the arguments are different,
    *   \li a dependency has been set between the two arguments,
    *   \li this conflict has been set yet.
    *   \sa Conflicts< T > with cascading.
    */
    void add_conflict(const std::string& arg1, const std::string& arg2);
    /*! \brief Removes the conflict between the given arguments.
    * 
    *   An assertion occurs if at least one of the argument name is unknown or if the conflict does not exist.
    */
    void remove_conflict(const std::string& arg1, const std::string& arg2);
    /*! \brief Removes all the conflicts involving the given argument.
    * 
    *   An assertion occurs if the argument name is unknown or if no existing conflict involves this argument.
    */
    void remove_conflicts(const std::string& argument);
    /*! \brief Clears all the existing conflicts. */
    void clear_conflicts() noexcept;
    /*! \brief Returns true if the given argument is in conflict with at least one other argument.
    * 
    *   An assertion occurs if the argument name is unknown.
    */
    bool in_conflict(const std::string& argument) const;
    /*! \brief Returns true if the given arguments are in conflict.
    * 
    *   An assertion occurs if at least one of the argument name is unknown.
    */
    bool in_conflict(const std::string& arg1, const std::string& arg2) const;
    /*! \brief Return a pointer to the argument arg2 in conflict with the given argument arg1.
    * 
    *   An assertion occurs if one of the argument name is unknown or if no conflict exists between the two arguments.
    */
    Argument* get_conflict(const std::string& arg1, const std::string& arg2);
    /*! \brief Returns the list of argument names in conflict with the given argument.
    * 
    *   An assertion occurs if the argument name is unknown.
    */
    std::vector<std::string> get_conflicts(const std::string& argument) const;
    /*! \brief Returns the whole list of argument name pairs in conflicts. */
    std::unordered_multimap<std::string, std::string> get_conflicts() const;
    /*! \brief Sets the conflicts from the given list of argument name pairs.
    * 
    *   An assertion occurs if rules are broken.
    *   \sa Usage::add_conflict()
    */
    void set_conflicts(const std::unordered_multimap<std::string, std::string>& conflicts);

    /*! \brief Loads the usage data from the given file.
        \todo Not yet implemented.
    */
    void load_from_file(const std::string& fname);      // TODO
    /*! \brief Saves the usage data to the given file.
    *   \todo Not yet implemented.
    */
    void save_to_file(const std::string& fname) const;  // TODO
    /*! \brief Sets the syntax of the program command line. */
    void set_syntax(const std::string& syntax);
    /*! \brief Returns true if the syntax string has been set. */
    bool syntax_is_valid() const noexcept { return m_syntax_valid; };
    /*! \brief Checks the arguments passed to the program and assign their values.
    * 
    *   Returns a message that indicates if the arguments were correctly parsed:
    *   \li an empty string indicates a success,
    *   \li the string "?" indicates that help display has been required,
    *   \li all other messages indicate the reason of the parsing fail.
    */
    std::string set_parameters(int argc, char* argv[]);
    // returns the collection of <Argument, value> pairs corresponding to argv, with control of rules
    // returns "" if succeed, or the error message if it fails

    /*! \brief Used to display the usage to the given output stream.
    * 
    *   Description is displayed first, then the command line syntax and arguments description.
    *   Finally the usage string is added to the output.
    */
    friend std::ostream& operator<<(std::ostream& os, Usage& us);
    // use to print usage help
};
