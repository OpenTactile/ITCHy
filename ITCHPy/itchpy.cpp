#include "itchpy.h"

#include <itchy/tactilemousequery.h>

p::str showQVector2D(QVector2D const& v) {
    p::str res = "<";
    res += p::object(v).attr("__class__").attr("__name__");
    res += "(" + p::str(v.x()) + "," + p::str(v.y()) + ")>";
    return res;
}

BOOST_PYTHON_MODULE(ITCHPy)
{
    p::class_<QVector2D>("QVector2D")
            .def(p::init<double, double>())
            .add_property("x", &QVector2D::x, &QVector2D::setX)
            .add_property("y", &QVector2D::y, &QVector2D::setY)
            .def("length", &QVector2D::length)
            .def("lengthSquared", &QVector2D::lengthSquared)
            .def("__repr__", &showQVector2D);

    p::class_<TactileMouseQuery>("TactileMouseQuery", p::init<p::optional<bool, unsigned int>>())
            .def("position", &TactileMouseQuery::position)
            .def("velocity", &TactileMouseQuery::velocity)
            .def("orientation", &TactileMouseQuery::orientation)
            .def("angularVelocity", &TactileMouseQuery::angularVelocity)
            .def("buttonPressed", &TactileMouseQuery::buttonPressed)
            .def("update", &TactileMouseQuery::update)
            .def("initialize", &TactileMouseQuery::initialize)
            .def("feedback", &TactileMouseQuery::feedback);
}
