require("./requireJS-node");
require.paths.unshift(__dirname + "/../lib");
require.paths.unshift(__dirname + "/cockpit/lib");
require.paths.unshift(__dirname + "/pilot/lib");
require.paths.unshift(__dirname + "/async/lib");
require.paths.unshift(__dirname + "/node-htmlparser/lib");
require.paths.unshift(__dirname + "/jsdom/lib");
require.paths.unshift(__dirname);