'use strict';

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

    if (!opts || !opts.log) {
        throw new Error('Router: cannot initialize with no opts');
    }

    this._log = opts.log;

    this._handlers = {};
    this._middleware = [];
}

/**
 * Create request object from raw data (byte array) and invoke handlers.
 */
Router.prototype.request = function (data, next) {
    this._log.trace({data: data}, 'Routing request');

    var packet = {
        route: null,
        data: data
    };

    this._invoke(packet, next);
};

Router.prototype.middleware = function middleware(fn) {
    this._log.trace({id: this._id}, 'Adding middleware');
    this._middleware.push(fn);
};

Router.prototype.route = function handler(route, fn) {
    this._log.trace({id: this._id, route: route}, 'Adding route');
    this._handlers[route] = fn;
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

    if (!handler) {
        this._log.warn({route: packet.route}, 'Route not found');
        next(new Error('Route not found'));
        return;
    }

    if (handler instanceof Router) {
        setImmediate(handler._invoke.bind(handler, packet, next));
        return;
    }

    if (typeof handler === 'function') {
        setImmediate(handler.bind(null, packet, next));
        return;
    }

    // TODO: invalid route error
};

module.exports = Router;
