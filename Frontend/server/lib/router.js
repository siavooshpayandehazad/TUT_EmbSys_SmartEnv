'use strict';

var extend = require('extend');

/**
 * Simple router to handle incoming data packets asynchronously.
 * @param opts
 * @returns {Router}
 * @constructor
 */

// TODO: proper error handling

function Router(opts) {
    if (!(this instanceof Router)) {
        return new Router(opts);
    }

    opts = (opts instanceof Router) ? opts._opts : opts;

    if (!opts || !opts.log) {
        throw new Error('Router: cannot initialize with no opts');
    }

    this._opts = opts;

    this._log = opts.log;

    this._defaultHandler = null;
    this._handlers = {};
    this._middleware = [];
}

/**
 * Create request object from raw data buffer and invoke handlers.
 */
Router.prototype.request = function (data, next) {
    this._log.trace({data: data}, 'Routing request');

    var packet = {
        raw: data,
        route: null
    };

    this._invoke(packet, next);
};

Router.prototype.middleware = function middleware(fn) {
    this._log.trace({id: this._id}, 'Adding middleware');
    this._middleware.push(fn);
};

Router.prototype.route = function handler(route, handler) {
    if (isHandlerType(route)) {
        handler = route;
        route = null;
    }

    if (!isHandlerType(handler)) {
        throw new TypeError('Expecting route handler to be Router object or function');
    }

    this._log.trace({id: this._id, route: route || '_default'}, 'Adding route');
    route ? this._handlers[route] = handler : this._defaultHandler = handler;
};

Router.prototype._invokeMiddleware = function _invokeMiddleware(req) {
    // Invoking all middleware.
    // NOTE: middleware is invoked synchronously, no heavy processing should be made.

    for (var i = 0; i < this._middleware.length; ++i) {
        this._middleware[i](req);
    }
};

Router.prototype._invoke = function _invoke(packet, next) {
    this._log.trace({packet: packet}, 'Invoking route');

    this._invokeMiddleware(packet);

    var handler = this._handlers[packet.route];
    var usingDefault = false;
    if (!handler) {
        handler =  this._defaultHandler;
        usingDefault = true;
    }

    if (!handler) {
        this._log.warn({route: packet.route}, 'Route not found');
        next(getRouteNotFoundError());
        return;
    }

    var _this = this;

    invokeHandler(handler, extend(true, {}, packet, beforeNext));

    function beforeNext(err) {
        if (err && err._routeNotFound && _this._defaultHandler && !usingDefault) {
            invokeHandler(_this._defaultHandler, extend(true, {}, packet), next);
            return;
        }

        next(err);
    }

    function invokeHandler(handler, packet, next) {
        if (handler instanceof Router) {
            setImmediate(handler._invoke.bind(handler, packet, next));
            return;
        }

        // Handler is function
        setImmediate(handler.bind(this, packet, next));
    }
};

module.exports = Router;


/* Helper functions */

function isHandlerType(handler) {
    return (handler instanceof Router) || (typeof handler === 'function');
}


function getRouteNotFoundError() {
    var err = new Error('Route not found');
    err._routeNotFound = true;
    return err;
}
