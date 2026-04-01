from setuptools import find_packages, setup

package_name = "robot_manager_interface"

setup(
    name=package_name,
    version="0.0.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
        ("share/" + package_name + "/launch", ["launch/interface.launch.py"]),
        ("share/" + package_name + "/templates", ["templates/index.html"]),
        (
            "share/" + package_name + "/static",
            [
                "static/style.css",
                "static/app.js",
            ],
        ),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="Ahmed",
    maintainer_email="engahmedhassan309@gmail.com",
    description="Web dashboard for robot manager",
    license="TODO: License declaration",
    tests_require=["pytest"],
    entry_points={
        "console_scripts": [
            "web_bridge = robot_manager_interface.web_bridge:main",
        ],
    },
)
