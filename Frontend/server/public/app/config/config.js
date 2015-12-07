'use strict';

(function (app) {
    app.config([
        '$locationProvider',
        '$stateProvider',
        '$urlRouterProvider',
        function ($locationProvider, $stateProvider, $urlRouterProvider) {

            //$locationProvider.html5Mode(true);
            $urlRouterProvider.otherwise('/log');

            $stateProvider
                .state('log', {
                    url: '/log',
                    templateUrl: 'app/views/log.html',
                    controller: 'LogController'
                });

        }
    ]);
})(angular.module('HAFE'));

