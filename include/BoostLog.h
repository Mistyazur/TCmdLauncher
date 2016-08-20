#pragma once
#ifndef _BOOSTLOG_H_
#define _BOOSTLOG_H_

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
#include <boost/serialization/singleton.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/attributes/current_process_name.hpp>

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;
namespace ser = boost::serialization;
namespace logging = boost::log;
namespace src = boost::log::sources;
namespace attr = boost::log::attributes;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;

enum SeverityLevel
{
	Trace,
	Debug,
	Info,
	Warning,
	Error,
	Fatal
};

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", SeverityLevel)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)

class BoostLog :
	public ser::singleton<BoostLog>
{
public:
	BoostLog()
	{
		// Get current path

		std::string currentPath(MAX_PATH, 0);
		::GetModuleFileNameA(NULL, const_cast<LPSTR>(currentPath.c_str()), MAX_PATH);
		currentPath = fs::path(currentPath).parent_path().string();
		currentPath.append("\\");

		// Read log settings

		std::string logSettingFile("Log.ini");
		logSettingFile = currentPath + logSettingFile;
		if (!fs::exists(logSettingFile))
			return;

		pt::ptree tree;
		pt::read_ini(logSettingFile, tree);
		bool enable = tree.get<bool>("Core.Enable", false);
		int debugLevel = tree.get("Core.Level", (int)Trace);

		// Check log switch

		if (!enable)
			return;

		// Setup log

		boost::shared_ptr< sinks::synchronous_sink< sinks::text_file_backend > > sink = logging::add_file_log(
			keywords::file_name = currentPath + "Log/%Y%m%d_%H%M%S%f_%N.log",
			keywords::open_mode = std::ios_base::app,
			keywords::format = (
				expr::stream
				<< expr::format_date_time(timestamp, "[%Y-%m-%d %H:%M:%S.%f]")
				<< "[" << expr::attr< logging::attributes::current_process_name::value_type > ("Process") << "]"
				<< "[L" << severity.or_default(Trace) << "] "
				<< expr::message
				),
			keywords::rotation_size = 10 * 1024 * 1024,
			keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
			keywords::auto_flush = true
		);

		// Add attributes

		logging::add_common_attributes();
		logging::core::get()->add_global_attribute("Process", attr::current_process_name());

		// Imbue sink with system locale instead of "zh_CHS"

		std::locale loc = boost::locale::generator()("");
		sink->imbue(loc);

		// Set level

		sink->set_filter(severity >= debugLevel);
	};

	~BoostLog()
	{
	};

	src::severity_logger< SeverityLevel > getSeverityLogger() const
	{
		return slg;
	};

	src::wseverity_logger< SeverityLevel > getSeverityLoggerW() const
	{
		return wslg;
	};
private:
	src::severity_logger< SeverityLevel > slg;
	src::wseverity_logger< SeverityLevel > wslg;
};

#define BLA(lvl) BOOST_LOG_SEV(BoostLog::get_const_instance().getSeverityLogger(), lvl)
#define BLW(lvl) BOOST_LOG_SEV(BoostLog::get_const_instance().getSeverityLoggerW(), lvl)

#endif
