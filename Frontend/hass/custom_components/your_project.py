"""
custom_components.hello_world
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Implements the bare minimum that a component should implement.
Configuration:
To use the hello_word component you will need to add the following to your
configuration.yaml file.
hello_world:
"""

# The domain of your component. Should be equal to the name of your component
DOMAIN = "your_project"

# List of component names (string) your component depends upon
DEPENDENCIES = []


def setup(hass, config):
    """ Setup our skeleton component. """

    # States are in the format DOMAIN.OBJECT_ID
    hass.states.set('your_project.Your_Project', 'Your project goes here!')

    # return boolean to indicate that initialization was successful
    return True
