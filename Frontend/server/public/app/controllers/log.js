'use strict';

(function (app) {
    app.controller('LogController', [
        '$http',
        '$scope',
        function ($http, $scope) {

            // Define variables and functions, Set default filter values

            $scope.showHelp = false;

            $scope.loading = false;
            $scope.error = false;

            $scope.logs = [];

            $scope.loadLogs = function loadLogs() {
                $scope.logs = [];
                $scope.loading = true;
                $scope.error = false;

                $http
                    .get('/api/debug/log', {
                        params: {
                            level: $scope.filter.level,
                            timeStart: $scope.filter.timeStart,
                            timeEnd: $scope.filter.timeEnd
                        }
                    })
                    .then(function (data) {
                        $scope.logs = data.data.map(function (log) {
                            log.level = levelByCode[log.level];
                            log.details = getLogDetailsJson(log);
                            return log;
                        });
                    })
                    .catch(function (err) {
                        console.error(err);
                        $scope.error = err.data || true;
                    })
                    .finally(function () {
                        $scope.loading = false;
                    });
            };

            $scope.levelOptions = ['trace', 'debug', 'info', 'warn', 'error'];

            $scope.timeOptions = [
                {value: '5', label: '5 min ago'},
                {value: '10', label: '10 min ago'},
                {value: '15', label: '15 min ago'},
                {value: '30', label: '30 min ago'},
                {value: '45', label: '45 min ago'},
                {value: '60', label: '1 h ago'},
                {value: '120', label: '2 h ago'}
            ];

            $scope.filter = {
                level: 'info',
                timeStart: null,
                timeEnd: null
            };

            $scope.setTimeFilter = function (name, mins) {
                mins = parseInt(mins, 10);
                $scope.filter[name] = mins ? new Date(Date.now() - mins * 60 * 1000).toISOString() : null;
            };

            $scope.timeSelect = {
                start: '60',
                end: ''
            };

            $scope.setTimeFilter('timeStart', $scope.timeSelect.start);
            $scope.setTimeFilter('timeEnd', $scope.timeSelect.end);

            // Log functions

            var levelByCode = {
                '10': 'trace',
                '20': 'debug',
                '30': 'info',
                '40': 'warn',
                '50': 'error'
            };

            var excludeKeys = ['$$hashKey', 'details', 'file', 'hostname', 'level', 'msg', 'name', 'pid', 'time', 'v'];
            function getLogDetailsJson(log) {
                var details = {};
                var hasDetails = false;

                Object.keys(log).forEach(function (key) {
                    if (excludeKeys.indexOf(key) !== -1) {
                        return;
                    }

                    details[key] = log[key];
                    hasDetails = true;
                });

                return hasDetails ? JSON.stringify(details, null, '\t').replace(/\n/g, '<br>').replace(/\t/g, '&#160;&#160;') : null;
            }

            // Make initial load

            $scope.loadLogs();
        }
    ]);
})(angular.module('HAFE'));
