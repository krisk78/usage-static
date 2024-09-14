#pragma once

/*! \file usage-static.hpp
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

namespace Usage
{

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
        /*! \brief Checks the type of the argument (named or unnamed).
        *   \return true if the argument is a named argument
        */
        virtual bool named() const noexcept = 0;

        /*! \brief Gets the name of the argument (set at instantiation).
        *   \return the name of the argument
        */
        std::string name() const noexcept;

        /*! \brief Sets or gets the help string (displayed in the usage output).
        *   \return the help string associated with the argument
        */
        std::string helpstring{};

        /*! \brief Checks if the argument is mandatory.
        *   \return true if the argument is mandatory
        *   \sa Argument::set_required()
        */
        bool required() const noexcept { return m_required; }

        /*! \brief List of values passed for the argument. Unnamed arguments can contain several values. */
        std::vector<std::string> value{};

        Argument() = delete;
        /*! \brief Default constructor that sets the name of the argument.
        *   \param name the name to give to the named argument
        */
        Argument(const std::string& name);

        /*! \brief Copy constructor by reference.
        *   \param argument the argument to copy
        */
        Argument(const Argument& argument);

        /*! \brief Copy constructor by pointer.
        *   \param argument the argument to copy
        */
        Argument(const Argument* argument);

        /*! \brief Sets if the argument is mandatory or not.
        *   \param required true to set the argument as mandatory, else false
        */
        virtual void set_required(const bool required) = 0;

        /*! \brief This override appends the xml definition of the argument to the output stream.
        *   \param os the output stream
        *   \param argument the argument for which the xml definition must be appended to the stream
        *   \return the modified output stream
        */
        friend std::ostream& operator<<(std::ostream& os, const Argument& argument);
        // used to return the xml definition of the Argument

    protected:
        /*! \brief Sets or gets the name of the argument. */
        std::string m_name{};

        /*! \brief Sets or gets the obligatory status of the argument, false by default. */
        bool m_required{ false };

        /*! \brief Prints in the output stream the help string of the argument.
        *   \param os the output stream
            \param indent optional string inserted before the argument help
            \return the modified output stream
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

        /*! \brief Indicates that the argument is a named argument.
        *   \return always true
        *   \sa Argument::named()
        *   \sa Unnamed_Arg::named()
        */
        virtual bool named() const noexcept override { return true; }

        /*! \brief Sets or gets the shortcut character. */
        char shortcut_char{ ' ' };

        /*! \brief Gets the type of the argument (string, boolean or simple).
        *   \return the type of the argument
        *   \sa Argument_Type
        */
        Argument_Type type() noexcept { return m_type; }

        /*! \brief Gets the default value applied when the argument is not used.
        *   \return a string containing the default value of the argument
        */
        std::string default_value() noexcept { return m_default_value; }

        Named_Arg() = delete;

        /*! \brief Default constructor that sets the name of the argument.
        *   \param name the name of the argument
        */
        Named_Arg(const std::string& name) : Argument(name) {};

        /*! \brief Copy constructor by reference.
        *   \param argument the argument to copy
        */
        Named_Arg(const Named_Arg& argument);

        /*! \brief Copy constructor by pointer. 
        *   \param argument the argument to copy
        */
        Named_Arg(const Named_Arg* argument);

        /*! \brief Sets if the argument is mandatory or not.
        *   \param required true to set the argument as mandatory, else false
        */
        virtual void set_required(const bool required) override;

        /*! \brief Sets the type (string, boolean or simple) of the named argument.
        *   \param type the type of the argument
        *   \sa Argument_Type
        *   \sa Argument::set_required()
        */
        void set_type(Argument_Type type);

        /*! \brief Sets the default value of the named argument.
        *   \param default_value a string containing the default value to apply to the argument
        */
        void set_default_value(const std::string& default_value);

    protected:
        /*! \brief Sets or gets the type of the argument. The argument type is simple by default.*/
        Argument_Type m_type{ Argument_Type::simple };

        /*! \brief Sets or gets the default value of the argument. */
        std::string m_default_value{};

        /*! \brief Prints in the output stream the help string of the argument.
        *   \param os the output stream
            \param indent optional string inserted before the argument help
            \return the modified output stream
            \sa Argument::print()
        */
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

        /*! \brief Indicates that the argument is a unnamed argument.
        *   \return always false
        *   \sa Argument::named()
        *   \sa Named_Arg::named()
        */
        virtual bool named() const noexcept override { return false; }

        /*! \brief Sets or gets the capability of the argument to accept many values. */
        bool many{ false };

        Unnamed_Arg() = delete;

        /*! \brief Default constructor that sets the name of the argument.
        *   \param name the name of the argument
        */
        Unnamed_Arg(const std::string& name) : Argument(name) {};

        /*! \brief Copy constructor by reference.
        *   \param argument the argument to copy
        */
        Unnamed_Arg(const Unnamed_Arg& argument) : Argument(argument) { many = argument.many; }

        /*! \brief Copy constructor by pointer.
        *   \param argument the argument to copy
        */
        Unnamed_Arg(const Unnamed_Arg* argument) : Argument(argument) { many = argument->many; }

        /*! \brief Sets if the argument is mandatory or not.
        *   \param required true to set the argument as mandatory, else false
        */
        virtual void set_required(const bool required) override { Argument::m_required = required; }

    protected:

        /*! \brief Prints in the output stream the help string of the argument.
        *   \param os the output stream
            \param indent optional string inserted before the argument help
            \return the modified output stream
            \sa Argument::print()
        */
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
        Requirements::Requirements<const Argument*> m_requirements{ false };

        // arguments delimited by | ; conflicting arguments must be either all required either all optional
        // if they are all required they are delimited by () ; if they are all optional they are delimited by []
        Conflicts::Conflicts<const Argument*> m_conflicts{ true };         // with cascading

        // command line syntax
        std::string m_syntax_string{};

        // build the command line syntax ; build_syntax is called each time the operator << is used
        //void create_syntax();               // TODO
        bool m_syntax_valid{ false };

        // process string that does not start with the switch char
        bool m_check_unnamed(const std::string& value, std::vector<bool>& set_args, bool& many, size_t& unnamed);

        // checks value type
        std::string m_check_type(const std::string& p, Argument_Type type_p, const std::string& value, std::vector<bool>& set_args);

        // parse the command line
        std::string m_parser(int argc, char* argv[], std::vector<bool>& set_args);

        // process requirements
        void m_check_requirements(size_t arg_index, std::vector<bool>& set_args);

        // process single argument
        size_t m_check_argument(std::vector<bool>& set_args);

        // check dependencies
        std::string m_check_dependencies(std::vector<bool>& set_args);

    public:

#ifdef _WIN32
        /*! \brief Gets the switch char used to start a named arg. */
        const char switch_char{ '/' };
#elif __unix__
        /*! \brief Gets the switch char used to start a named arg. */
        const char switch_char{ '-' };
#endif

#ifdef _WIN32
        /*! \brief Gets the help argument. */
        const std::string help_arg{ "?" };
#elif __unix__
        /*! \brief Gets the help argument. */
        const std::string help_arg{ "h" };
#endif

        /*! \brief Sets or gets the name of the program.*/
        std::string program_name{};
        /*! \brief Sets or gets the brief description of the program that is displayed before the syntax and arguments explanation. */
        std::string description{};
        /*! \brief Sets or gets the help text that is displayed after the syntax and arguments explanation, including examples. */
        std::string usage{};

        // A mandatory argument must be passed to the constructor
        Usage() = delete;

        /*! \brief Default constructor that sets the program name.
        *   \param prog_name the name of the executable that uses this class to manage its arguments
            \note The absolute path of the executable is present in the first argument argv[0] passed to the main function.
        */
        Usage(const std::string& prog_name);    // program name is in argv[0] passed to main()

        /*! \brief Destructor. */
        ~Usage();

        /*! \brief Adds an argument to the list of arguments.
        *   \param argument the named or unnamed argument to add
            \warning An assertion occurs if an argument with the same name already exists.
        */
        void add_Argument(const Argument& argument);

        /*! \brief Removes the argument of the list by its name.
        *   \param name remove an argument given by its name
            \warning An assertion occurs if the argument name is unknown.
        */
        void remove_Argument(const std::string& name);

        /*! \brief Removes all arguments from the list. */
        void remove_all() noexcept;

        /*! \brief Removes all arguments from the list and clears the description, syntax and usage help strings. */
        void clear() noexcept;

        /*! \brief Search an argument by its name.
        *   \param name the name of the argument to search for
        *   \return a pointer to the argument if found else nullptr
        */
        Argument* get_Argument(const std::string& name);

        /*! \brief Lists arguments.
        *   \return the list of pointers to arguments
        */
        std::vector<Argument*> get_Arguments();

        /*! \brief Lists all arguments with their assigned values.
        *   \return the list of argument names with their assigned value(s)
        */
        std::unordered_map<std::string, std::vector<std::string>> get_values() const;       // return couples name, values - to use after call of set_parameters

        /*! \brief Lists values passed through the command line for the requested argument name.
        *   \param name the name of the requested argument
        *   \return the list of values assigned to the argument
            \warning An assertion occurs if the argument name is unknown.
        */
        std::vector<std::string> get_values(const std::string& name) const;                 // return values for a single argument

        /*! \brief Sets a dependency between 2 arguments.
        *   \param dependent the name of the argument that requires, to be used, that any other argument is set
        *   \param requirement the name of the argument on which dependent depends
            \warning An assertion occurs if:
            \li one of the two argument name is unknown,
            \li the two arguments are same,
            \li a conflict has yet been set between these arguments,
            \li this requirement has been set yet.
            \sa Requirements< T > without reflexivity.
        */
        void add_requirement(const std::string& dependent, const std::string& requirement);

        /*! \brief Removes the dependency between 2 arguments.
        *   \param dependent,requirement the 2 argument names that are dependent
            \warning An assertion occurs if one of the two argument name is unknown or if the requirement does not exist.
        */
        void remove_requirement(const std::string& dependent, const std::string& requirement);

        /*! \brief Removes all the requirements for the given argument.
        *   \param dependent the name of the argument for which requirement relationships must be removed
        *   \warning An assertion occurs if the argument name is unknown or if no requirement is found.
        */
        void remove_requirements(const std::string& dependent);

        /*! \brief Clears all existing requirement relationships. */
        void clear_requirements() noexcept;

        /*! \brief Checks if a dependency exists between 2 arguments.
        *   \param dependent,requirement the names of the 2 arguments for which the dependency is searched for
        *   \return true if the dependency exists
        *   \warning An assertion occurs if one of the argument name is unknown.
        */
        bool requirement_exists(const std::string& dependent, const std::string& requirement) const;

        /*! \brief Checks if an argument is dependent from other arguments.
        *   \param dependent the name of the argument for which requirement relationships is searched for
        *   \return true if the argument is dependent from at least one other argument
        *   \warning An assertion occurs if the argument name is unknown.
        */
        bool has_requirements(const std::string& dependent) const;

        /*! \brief Checks if some arguments are dependent from an argument.
        *   \param requirement the name of the argument for which dependents are searched for
        *   \return true if at least one argument is dependent from the given argument
        *   \warning An assertion occurs if the argument name is unknown.
        */
        bool has_dependents(const std::string& requirement) const;

        /*! \brief Gets the required argument of a dependency, searched by argument names.
        *   \param dependent,requirement the name of the 2 arguments of the dependency
        *   \return a pointer to the required argument 
        *   \warning An assertion occurs if one of the argument name is unknown or if the dependency does not exist.
        */
        Argument* get_requirement(const std::string& dependent, const std::string& requirement);

        /*! \brief Lists argument names that are required by the given argument.
        *   \param dependent the name of the argument for which required arguments are searched for
        *   \return the list of argument names on which the given argument depends
        *   \warning An assertion occurs if the argument name is unknown.
        */
        std::vector<std::string> get_requirements(const std::string& dependent) const;

        /*! \brief Lists the names of the arguments that depends on the given argument.
        *   \param requirement the name of the argument for which dependents are searched for
        *   \return the list of argument names that depends on the given argument
        *   \warning An assertion occurs if the argument name is unknown.
        */
        std::vector<std::string> get_dependents(const std::string& requirement) const;

        /*! \brief Lists the pairs (dependent, requirement) of argument names involved in a dependency
        *   \return the list of name pairs
        */
        std::unordered_multimap<std::string, std::string> get_requirements() const;

        /*! \brief Sets the dependencies between arguments from the given list of name pairs (dependent, requirement).
        *   \param requirements the list of name pairs for which a dependency must be created
        *   \warning An assertion occurs if rules are broken.
        *   \sa Usage::add_requirement()
        */
        void set_requirements(const std::unordered_multimap<std::string, std::string>& requirements);

        /*! \brief Adds a conflict between the given arguments.
        *   \param arg1,arg2 the names of the arguments in conflict
        * 
            Arguments in conflict are delimited by '|' in the command syntax. They cannot be used together.
        *   \warning An assertion occurs if:
        *   \li at least one of the argument name is unknown,
        *   \li arguments are same,
        *   \li the required status of the arguments are different,
        *   \li a dependency has been set between the two arguments,
        *   \li this conflict has been set yet.
        *   \sa Conflicts< T > with cascading.
        */
        void add_conflict(const std::string& arg1, const std::string& arg2);

        /*! \brief Removes the conflict between the given arguments.
        *   \param arg1,arg2 the names of the arguments in conflict
        *   \warning An assertion occurs if at least one of the argument name is unknown or if the conflict does not exist.
        */
        void remove_conflict(const std::string& arg1, const std::string& arg2);

        /*! \brief Removes all the conflicts involving the given argument.
        *   \param argument the name of the argument for which all conflict relationships must be removed
        *   \warning An assertion occurs if the argument name is unknown or if no existing conflict involves this argument.
        */
        void remove_conflicts(const std::string& argument);

        /*! \brief Clears all the existing conflicts. */
        void clear_conflicts() noexcept;

        /*! \brief Checks if the given argument is in conflict with other argument.
        *   \param argument the name of the argument for which conflict relationships are searched for
        *   \return true if at least one argument is in conflict with the given argument
        *   \warning An assertion occurs if the argument name is unknown.
        */
        bool in_conflict(const std::string& argument) const;

        /*! \brief Checks if 2 arguments are in conflict.
        *   \param arg1,arg2 the names of the arguments for which a conflict relationship is searched for
        *   \return true if the arguments are in conflict
        *   \warning An assertion occurs if at least one of the argument name is unknown.
        */
        bool in_conflict(const std::string& arg1, const std::string& arg2) const;

        /*! \brief Gets the 2nd argument of a conflict relatioship given by the arguments names.
        *   \param arg1,arg2 the names of the arguments in conflict
        *   \return a pointer to the 2nd argument arg2
        *   \warning An assertion occurs if one of the argument name is unknown or if no conflict exists between the two arguments.
        */
        Argument* get_conflict(const std::string& arg1, const std::string& arg2);

        /*! \brief Lists the arguments in conflict with the given argument.
        *   \param argument the name of the argument for which conflicts are searched for
        *   \return the list of arguments names in conflict with the given argument
        *   \warning An assertion occurs if the argument name is unknown.
        */
        std::vector<std::string> get_conflicts(const std::string& argument) const;

        /*! \brief Lists all arguments in conflicts.
        *   \return a list of argument name pairs in conflict
        */
        std::unordered_multimap<std::string, std::string> get_conflicts() const;

        /*! \brief Sets the conflicts from the given list.
        *   \param conflicts the list of argument name pairs for which conflicts must be set
        *   \warning An assertion occurs if rules are broken.
        *   \sa Usage::add_conflict()
        */
        void set_conflicts(const std::unordered_multimap<std::string, std::string>& conflicts);

        /*! \brief Loads the usage data from the given file.
        *   \param fname the name of the file containing the data
            \todo Not yet implemented.
        */
        void load_from_file(const std::string& fname);      // TODO

        /*! \brief Saves the usage data to the given file.
        *   \param fname the name of the file to save
        *   \todo Not yet implemented.
        */
        void save_to_file(const std::string& fname) const;  // TODO

        /*! \brief Sets the syntax of the command line.
        *   \param syntax the string representing the syntax of the command line
        *   \todo Build dependencies and conflicts from the command line syntax
        */
        void set_syntax(const std::string& syntax);

        /*! \brief Checks if the syntax string has been set.
        *   \return true if the syntax of the command line has been set
        */
        bool syntax_is_valid() const noexcept { return m_syntax_valid; };

        /*! \brief Checks the arguments passed to the program and assign their values.
        *   \param argc the number of arguments passed to the main executable
        *   \param argv an array of c-like strings passed to the main executable
        *   \return a message that indicates if the arguments were correctly parsed:
        *   \li an empty string indicates a success,
        *   \li the string "?" indicates that usage help has been required,
        *   \li all other messages indicate the reason of the parsing fail.
        */
        std::string set_parameters(int argc, char* argv[]);
        // returns the collection of <Argument, value> pairs corresponding to argv, with control of rules
        // returns "" if succeed, or the error message if it fails

        /*! \brief Append the usage help to the given output stream.
        *   \param os the output stream
        *   \param us the usage instance for which the usage help must be appended
        * 
        *   Description is displayed first, then the command line syntax and arguments description.
        *   Finally the usage string is added to the output.
        */
        friend std::ostream& operator<<(std::ostream& os, Usage& us);
        // use to print usage help
    };

}
