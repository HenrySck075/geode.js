#include "console.hpp"
#include <Geode/loader/Types.hpp>
#include "state.hpp"
#include <Geode/DefaultInclude.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/ModMetadata.hpp>
#include <chrono>
#include <iomanip>
#include <matjson.hpp>
#include <numeric>
#include <sstream>
#include <string>

inline void fireConsoleEvent(
  std::string type
) {
  ConsoleEvent(type).post();
};

geode::Mod* representer;

static void pushLogStr(std::string const& msg, geode::Severity severity) {
  geode::log::logImpl(severity, representer, "{}", msg);
}
static void pushLog(CFunctionsScopePtr const& msg, geode::Severity severity) {
  std::string j;
  if (!msg->getArgument(0)->isString()) {
    int argsLen = msg->getArgumentsLength();
    for (int i = 0; i < argsLen; i++) {
      j += msg->getArgument(i)->toString() + " ";
    }
  } else {
    j = msg->getArgument(0)->toString();
  }
  pushLogStr(j, severity);
  msg->setReturnVar({newScriptVarUndefined(getState())});
}
$jsMethod(Console_assert) {
  if (!v->getArgument(0)->toBoolean()) {
    auto j = v->findChild(TINYJS_ARGUMENTS_VAR)->getVarPtr();
    auto l = j->findChild(int2string(0));
    j->removeLink(l);

    pushLog(v, geode::Severity::Error);
  }
  fireConsoleEvent("assert");
}

static std::map<std::string, int64_t> counts;
$jsMethod(Console_count) {
  auto arg = v->getArgument(0);
  std::string label;
  if (arg->isUndefined()) label = "default";
  else label = arg->toString();

  counts[label]+=1;
  pushLogStr(label+": "+std::to_string(counts[label]), geode::Severity::Info);
  fireConsoleEvent("count");
}

$jsMethod(Console_log) {
  pushLog(v, geode::Severity::Info);
  fireConsoleEvent("log");
}
$jsMethod(Console_debug) {
  pushLog(v, geode::Severity::Debug);
  fireConsoleEvent("debug");
}
$jsMethod(Console_warn) {
  pushLog(v, geode::Severity::Warning);
  fireConsoleEvent("warn");
}
$jsMethod(Console_error) {
  pushLog(v, geode::Severity::Error);
  fireConsoleEvent("error");
}

$jsMethod(Console_group) {
  geode::log::pushNest(representer);
  fireConsoleEvent("startGroup");
}
$jsMethod(Console_groupEnd) {
  geode::log::popNest(representer);
  fireConsoleEvent("endGroup");
}

#include <cstdlib>
$jsMethod(Console_table) {
  std::vector<std::vector<std::string>> table;
  // max string length per cell on all rows
  std::vector<int> maxstrlen{7, 0};

  int rowlen = 1;
  if (v->isArray()) {
    int cc = v->getArrayLength();

    for (int i = 0; i < cc; i++) {
      std::vector<std::string> row{std::to_string(i)};
      auto c = v->getArrayIndex(i);

      if (c->isArray()) {
        int scc = c->getArrayLength();
        int oldrl = rowlen;
        rowlen = std::max(rowlen, scc);
        maxstrlen.reserve(rowlen);
        std::fill(maxstrlen.begin()+oldrl, maxstrlen.end(), 0);
        row.reserve(scc+1);

        for (int j = 0; j < scc; j++) {
          std::stringstream idk;
          idk << std::quoted(
            c->getArrayIndex(j)->toString(), '\'', '\\'
          );
          auto s = idk.str();
          if (maxstrlen[j+1]<s.size()) 
            maxstrlen[j+1] = s.size();
          row.push_back(s);
        }

      } else if (c->isPrimitive()) {
        std::stringstream ba;
        ba << std::quoted(
          c->toString(), '\'', '\\'
        );
        auto s = ba.str();
        if (maxstrlen[1]<s.size()) 
          maxstrlen[1] = s.size();
        row.push_back(s);
      }

      table.push_back(row);
    }
  } else if (!v->isPrimitive()) {}

  std::vector<std::string> r{"(index)"};
  r.reserve(rowlen+1);
  for (int i = 0; i < rowlen; i++) {
    r.push_back(std::to_string(i));
  }

  table.insert(table.begin(), r);

  int totallen = std::reduce(maxstrlen.begin(), maxstrlen.end()) + maxstrlen.size()-1;
  std::stringstream out;
  std::string sep(totallen, '-');
  out << '\n';
  out << sep << "--" << '\n';
  int tl = table.size();
  for (int ridx = 0; ridx < tl; ridx++) {
    auto& row = table[ridx];
    out << '|';
    int rl = row.size();
    for (int cidx = 0; cidx < rowlen; cidx++) {
      std::string cc;
      if (cidx<rl) cc = row[cidx];
      else cc = "";
      out << cc << std::string(maxstrlen[cidx],cc.size()) << '|';
    }
    if (ridx != tl-1) 
      out << "\n|" << sep << "|\n";
    else 
      out << '\n' << sep << "--" << '\n';
  }
  
  pushLogStr(out.str(), geode::Severity::Info);
}

static std::map<std::string, std::chrono::milliseconds> timers;

$jsMethod(Console_time) {
  auto arg = v->getArgument(0);
  std::string label;
  if (arg->isUndefined()) label = "default";
  else label = arg->toString();

  timers[label] = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()
  );
  v->setReturnVar(newScriptVarUndefined(getState()));
}
static void Console_timeLog_impl(CFunctionsScopePtr const& v, bool end) {
  auto arg = v->getArgument(0);
  std::string label;
  if (arg->isUndefined()) label = "default";
  else label = arg->toString();

  pushLogStr(
    label+": "+std::to_string((
      timers[label] - std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
      )
    ).count())+"ms"
      +(end?" - timer ended":""), 
    geode::Severity::Info
  );

  v->setReturnVar(newScriptVarUndefined(getState()));
}
$jsMethod(Console_timeLog) {
  Console_timeLog_impl(v, false);
}
$jsMethod(Console_timeEnd) {
  auto arg = v->getArgument(0);
  std::string label;
  if (arg->isUndefined()) label = "default";
  else label = arg->toString();

  Console_timeLog_impl(v, true);
  timers.erase(label);
}

$execute{
  /*
  geode::ModMetadata meta("henrysck075.puppeteer.js");

  meta.setName("JavaScript");
  meta.setVersion(geode::VersionInfo{7,7,7});
  meta.setDescription("The representation of the JavaScript console output from GD DevTools Protocol.");
  meta.setDevelopers(geode::Mod::get()->getDevelopers());
*/
  representer = geode::Mod::get();

  auto s = getState();
  {
    auto console = s->getRoot()->addChild("console", newScriptVar(s, Object))->getVarPtr();
    console->addChild("assert", newScriptVar(s, Console_assert, 0, "console.assert"));
    console->addChild("log", newScriptVar(s, Console_log, 0, "console.log"));
    console->addChild("info", newScriptVar(s, Console_log, 0, "console.info"));
    console->addChild("debug", newScriptVar(s, Console_debug, 0, "console.debug"));
    console->addChild("warn", newScriptVar(s, Console_warn, 0, "console.warn"));
    console->addChild("error", newScriptVar(s, Console_error, 0, "console.error"));
    
    console->addChild("group", newScriptVar(s, Console_group, 0, "console.group"));
    console->addChild("groupCollapsed", newScriptVar(s, Console_group, 0, "console.groupCollapsed")); // trolled
    console->addChild("groupEnd", newScriptVar(s, Console_groupEnd, 0, "console.groupEnd"));

    console->addChild("table", newScriptVar(s, Console_table, 0, "console.table"));

    console->addChild("time", newScriptVar(s, Console_time, 0, "console.time"));
    console->addChild("timeLog", newScriptVar(s, Console_timeLog, 0, "console.timeLog"));
    console->addChild("timeEnd", newScriptVar(s, Console_timeEnd, 0, "console.timeEnd"));
  }
}
