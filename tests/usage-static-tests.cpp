#include <gtest/gtest.h>
#include <usage-static.hpp>

class UsageTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		Usage::Unnamed_Arg files{ "file" };
		files.set_required(true);
		files.many = true;
		files.helpstring = "File(s) to compute.";
		us.add_Argument(files);
		Usage::Named_Arg o{ "extension" };
		o.set_type(Usage::Argument_Type::string);
		o.shortcut_char = 'o';
		o.set_default_value("sor.txt");
		o.helpstring = "Extension of the output file.";
		us.add_Argument(o);
		Usage::Named_Arg s{ "field_separator" };
		s.set_type(Usage::Argument_Type::string);
		s.shortcut_char = 's';
		s.set_default_value("\t");
		s.helpstring = "Field separator.";
		us.add_Argument(s);
		Usage::Named_Arg n{ "decimal_separator" };
		n.set_type(Usage::Argument_Type::string);
		n.shortcut_char = 'n';
		n.set_default_value(".");
		n.helpstring = "Decimal separator.";
		us.add_Argument(n);
		Usage::Named_Arg d{ "date_format" };
		d.set_type(Usage::Argument_Type::string);
		d.shortcut_char = 'd';
		d.set_default_value("d.m.y");
		d.helpstring = "Date format (use d for days, m for months and y for years).";
		us.add_Argument(d);
		Usage::Named_Arg p{ "position" };
		p.set_type(Usage::Argument_Type::string);
		p.set_required(true);
		p.shortcut_char = 'p';
		p.helpstring = "Number(s) of the field(s) to sort, separated by comma ','.";
		us.add_Argument(p);
		Usage::Named_Arg f{ "fixed" };
		f.set_type(Usage::Argument_Type::string);
		f.set_required(true);
		f.shortcut_char = 'f';
		f.helpstring = "Position(s) in chars and length(s) of the field(s) to sort, separated by comma ','.\n"
			"Letter L is used to separate position and length of a field.";
		us.add_Argument(f);
		Usage::Named_Arg r{ "reverse" };
		r.set_type(Usage::Argument_Type::simple);
		r.shortcut_char = 'r';
		r.helpstring = "Apply a descending sort instead of ascending sort.";
		us.add_Argument(r);
		Usage::Named_Arg b{ "begin" };
		b.set_type(Usage::Argument_Type::string);
		b.shortcut_char = 'b';
		b.set_default_value("1");
		b.helpstring = "Number of the starting row of the sort.";
		us.add_Argument(b);
		us.add_requirement("field_separator", "position");
		us.add_conflict("position", "fixed");
		us.description = "Sort files based on the specified keys.";
		//us.set_syntax("Usage.exe file... [/o:extension] [/n:decimal_separator] [/d:date_format] [/r] [/b:begin]\n"
		//				"          ([/s:field_separator] /p:position | /f:fixed)");

	}

	// void TearDown() override {}

	Usage::Usage us{ "program.exe" };

};

TEST(Named_ArgDeathTest, Set_Required_While_Default_Value)
{
	Usage::Named_Arg b{ "begin" };
	b.set_type(Usage::Argument_Type::string);
	b.set_default_value("any");
	EXPECT_DEATH(b.set_required(true), "");
}

TEST(Named_ArgDeathTest, Set_Type_Simple_While_Default_Value)
{
	Usage::Named_Arg b{ "begin" };
	b.set_type(Usage::Argument_Type::string);			// Needed because Named_Args are simple by default
	b.set_default_value("any");
	EXPECT_DEATH(b.set_type(Usage::Argument_Type::simple), "");
}

TEST(Named_ArgDeathTest, Set_Default_Value_For_Required_Argument)
{
	Usage::Named_Arg b{ "begin" };
	b.set_type(Usage::Argument_Type::string);
	b.set_required(true);
	EXPECT_DEATH(b.set_default_value("any"), "");
}

TEST(Named_ArgDeathTest, Set_Default_Value_For_Simple_Argument)
{
	Usage::Named_Arg b{ "begin" };
	b.set_type(Usage::Argument_Type::simple);
	EXPECT_DEATH(b.set_default_value("any"), "");
}

using UsageDeathTest = UsageTest;

TEST_F(UsageDeathTest, Add_Existing_Argument)
{
	Usage::Named_Arg b{ "begin" };
	b.set_type(Usage::Argument_Type::boolean);
	EXPECT_DEATH(us.add_Argument(b), "");
}

TEST_F(UsageDeathTest, Remove_Unknown_Argument)
{
	EXPECT_DEATH(us.remove_Argument("z"), "");
}

TEST_F(UsageDeathTest, Bad_Add_Requirement_Usage)
{
	EXPECT_DEATH(us.add_requirement("", ""), "");
}

TEST_F(UsageDeathTest, Add_Requirement_On_Itself)
{
	EXPECT_DEATH(us.add_requirement("reverse", "reverse"), "");
}

TEST_F(UsageDeathTest, Add_Requirement_On_Unknown_Argument)
{
	EXPECT_DEATH(us.add_requirement("reverse", "z"), "");
}

TEST_F(UsageDeathTest, Add_Requirement_On_Arguments_In_Conflict)
{
	EXPECT_DEATH(us.add_requirement("position", "fixed"), "");
}

TEST_F(UsageDeathTest, Add_Existing_Requirement)
{
	EXPECT_DEATH(us.add_requirement("field_separator", "position"), "");
}

TEST_F(UsageDeathTest, Remove_Unknown_Requirement)
{
	EXPECT_DEATH(us.remove_requirement("position", "fixed"), "");
}

TEST_F(UsageDeathTest, Bad_Add_Conflict_Usage)
{
	EXPECT_DEATH(us.add_conflict("", ""), "");
}

TEST_F(UsageDeathTest, Add_Conflict_With_Itself)
{
	EXPECT_DEATH(us.add_conflict("reverse", "reverse"), "");
}

TEST_F(UsageDeathTest, Add_Conflict_With_Unknown_Argument)
{
	EXPECT_DEATH(us.add_conflict("reverse", "z"), "");
}

TEST_F(UsageDeathTest, Add_Conflict_With_Required_Argument)
{
	EXPECT_DEATH(us.add_conflict("field_separator", "position"), "");
}

TEST_F(UsageDeathTest, Add_Conflict_With_Dependent_Argument)
{
	EXPECT_DEATH(us.add_conflict("position", "field_separator"), "");
}

TEST_F(UsageDeathTest, Add_Existing_Conflict)
{
	EXPECT_DEATH(us.add_requirement("fixed", "position"), "");
}

TEST_F(UsageDeathTest, Remove_Unknown_Conflict)
{
	EXPECT_DEATH(us.remove_requirement("reverse", "fixed"), "");
}

TEST_F(UsageTest, Set_Parameters0)
{
#if _WIN32
	std::string expected_str{ "Missing required argument 'file' - see program.exe /? for help." };
#elif __unix__
	std::string expected_str{ "Missing required argument 'file' - see program.exe -h for help." };
#endif
	std::vector<char*> argv{ "program.exe" };
	auto msg = us.set_parameters((int)argv.size(), &argv[0]);
	EXPECT_STREQ(msg.c_str(), expected_str.c_str());
}

TEST_F(UsageTest, Set_Parameters1)
{
#if _WIN32
	std::string expected_str{ "Missing required argument 'position' - see program.exe /? for help." };
#elif __unix__
	std::string expected_str{ "Missing required argument 'position' - see program.exe -h for help." };
#endif
	std::vector<char*> argv{ "program.exe", "files*.txt" };
	auto msg = us.set_parameters((int)argv.size(), &argv[0]);
	EXPECT_STREQ(msg.c_str(), expected_str.c_str());
}

TEST_F(UsageTest, Set_Parameters2)
{
#ifdef _WIN32
	std::string expected_str{ "Missing required argument 'position' - see program.exe /? for help." };
	std::vector<char*> argv{ "program.exe", "files*.txt", "/s:\",\"", "/f:3,7" };
#elif __unix__
	std::string expected_str{ "Missing required argument 'position' - see program.exe -h for help." };
	std::vector<char*> argv{ "program.exe", "files*.txt", "-s:\",\"", "-f:3,7" };
#endif
	auto msg = us.set_parameters((int)argv.size(), &argv[0]);
	EXPECT_STREQ(msg.c_str(), expected_str.c_str());
}

TEST_F(UsageTest, Set_Parameters3)
{
#ifdef _WIN32
	std::string expected_str{ "Argument 'reverse' passed as 'string' while expected type is 'simple' - see program.exe /? for help." };
	std::vector<char*> argv{ "program.exe", "files*.txt", "/r:2", "/f:3,7" };
#elif __unix__
	std::string expected_str{ "Argument 'reverse' passed as 'string' while expected type is 'simple' - see program.exe -h for help." };
	std::vector<char*> argv{ "program.exe", "files*.txt", "-r:2", "-f:3,7" };
#endif
	auto msg = us.set_parameters((int)argv.size(), &argv[0]);
	EXPECT_STREQ(msg.c_str(), expected_str.c_str());
}

TEST_F(UsageTest, Set_Parameters4)
{
#ifdef _WIN32
	std::string expected_str{ "Unknown argument '/z' - see program.exe /? for help." };
	std::vector<char*> argv{ "program.exe", "files*.txt", "/z", "/f:3,7" };
#elif __unix__
	std::string expected_str{ "Unknown argument '-z' - see program.exe -h for help." };
	std::vector<char*> argv{ "program.exe", "files*.txt", "-z", "-f:3,7" };
#endif
	auto msg = us.set_parameters((int)argv.size(), &argv[0]);
	EXPECT_STREQ(msg.c_str(), expected_str.c_str());
}

TEST_F(UsageTest, Set_Parameters5)
{
#ifdef _WIN32
	std::string expected_str{ "Arguments 'position' and 'fixed' can't be used together - see program.exe /? for help." };
	std::vector<char*> argv{ "program.exe", "files*.txt", "/p:2", "/f:3,7" };
#elif __unix__
	std::string expected_str{ "Arguments 'position' and 'fixed' can't be used together - see program.exe -h for help." };
	std::vector<char*> argv{ "program.exe", "files*.txt", "-p:2", "-f:3,7" };
#endif
	auto msg = us.set_parameters((int)argv.size(), &argv[0]);
	EXPECT_STREQ(msg.c_str(), expected_str.c_str());
}

TEST_F(UsageTest, Set_Parameters6)
{
#ifdef _WIN32
	std::string expected_str{ "Error found in command line argument number 2: '/z+\"2\"' - see program.exe /? for help." };
#elif __unix__
	std::string expected_str{ "Error found in command line argument number 2: '-z+\"2\"' - see program.exe -h for help." };
#endif
	Usage::Named_Arg z{ "z" };
	z.set_type(Usage::Argument_Type::boolean);
	us.add_Argument(z);
#ifdef _WIN32
	std::vector<char*> argv{ "program.exe", "files*.txt", "/z+\"2\"", "/f:3,7" };
#elif __unix__
	std::vector<char*> argv{ "program.exe", "files*.txt", "-z+\"2\"", "-f:3,7" };
#endif
	auto msg = us.set_parameters((int)argv.size(), &argv[0]);
	EXPECT_STREQ(msg.c_str(), expected_str.c_str());
}

TEST_F(UsageTest, Set_Parameters7)
{
#ifdef _WIN32
	std::vector<char*> argv{ "program.exe", "/?" };
#elif __unix__
	std::vector<char*> argv{ "program.exe", "-h" };
#endif
	auto msg = us.set_parameters((int)argv.size(), &argv[0]);
	EXPECT_STREQ(msg.c_str(), "?");
}

TEST_F(UsageTest, Set_Parameters8)
{
#ifdef _WIN32
	std::vector<char*> argv{ "program.exe", "\\files*.txt", "/f:3,7", "/r", "/n:\",\""};
#elif __unix__
	std::vector<char*> argv{ "program.exe", "/files*.txt", "-f:3,7", "-r", "-n:\",\"" };
#endif
	auto msg = us.set_parameters((int)argv.size(), &argv[0]);
	EXPECT_STREQ(msg.c_str(), "");
}
