"""This example shows a streaming dashboard with Panel.

Panel runs on top of the Tornado server. Tornado is a fast, asynchronous web server built to
support streaming use cases.

In panel it's very easy to support periodic updates. Here it's done via
`pn.state.add_periodic_callback(_create_callback(indicators), period=1000, count=200)`

This Dashboard is work-in-progress. I would like to add some different types of stats cards
including some with splines/ plots. I would also like to add some icons to make it look nice.
"""
from typing import List, Tuple
import os
import plotly.express as px
import numpy as np
import panel as pn
import pandas as pd
from datetime import date, datetime
import holoviews as hv

from awesome_panel import config

STYLE = """
.pn-stats-card div {
  line-height: 1em;
}
"""

SENSOR_NAMES = [
    "TDS 0","TDS 1","Water Height 0","Water Height 1",
    "Water Height 2","Water Height 3","Turbidity 0",
    "Turbidity 1","pH 0","pH 1","AQS 0","AQS 1","AQS 2",
    "CO2 0","CO2 1","CO2 2","Temp 0","Temp 1","Temp 2",
    "Humidity 0","Humidity 1","Humidity 2","Particle Count 0",
    "Particle Count 1","Particle Count 2","O2 0","O2 1","O2 2"
]
# THERE CAN BE NO CONFLICTS IN THE TITLES*** OR NAMES***** OF THE SENSORS
SENSOR_TITLES = {
    "TDS 0": "TDS Outside",
    "TDS 1": "TDS Inside",
    "Water Height 0": "Water Tank Left",
    "Water Height 1": "Water Tank Right",
    "Water Height 2": "Water Tank Back",
    "Water Height 3": "Water Tank Front",
    "Turbidity 0": "Turbidity Outside",
    "Turbidity 1": "Turbidity Inside",
    "pH 0": "pH Outside",
    "pH 1": "pH Inside",
    "AQS 0": "AQS Inside",
    "AQS 1": "AQS Greenhouse",
    "AQS 2": "AQS Front Porch",
    "CO2 0": "CO2 Outside",
    "CO2 1": "CO2 Inside",
    "CO2 2": "CO2 Basement",
    "Temp 0": "Temp Outside",
    "Temp 1": "Temp Inside",
    "Temp 2": "Temp Basement",
    "Humidity 0": "Humidity Outside",
    "Humidity 1": "Humidity Inside",
    "Humidity 2": "Humidity Basement",
    "Particle Count 0": "Particle Count Outside",
    "Particle Count 1": "Particle Count Inside",
    "Particle Count 2": "Particle Count Basement",
    "O2 0": "O2 Outside",
    "O2 1": "O2 Inside",
    "O2 2": "O2 Basement",
}

# SENSOR_TITLES = {
#     "TDS Outside": "TDS 0",
#     "TDS Inside": "TDS 1",
#     "Water Tank Left": "Water Height 0",
#     "Water Tank Right": "Water Height 1",
#     "Water Tank Back": "Water Height 2",
#     "Water Tank Front": "Water Height 3",
#     "Turbidity Outside": "Turbidity 0",
#     "Turbidity Inside": "Turbidity 1",
#     "pH Outside": "pH 0",
#     "pH Inside": "pH 1",
#     "AQS Inside": "AQS 0",
#     "AQS Greenhouse": "AQS 1",
#     "AQS Front Porch": "AQS 2",
#     "CO2 Outside": "CO2 0",
#     "CO2 Inside": "CO2 1",
#     "CO2 Basement": "CO2 2",
#     "Temp Outside": "Temp 0",
#     "Temp Inside": "Temp 1",
#     "Temp Basement": "Temp 2",
#     "Humidity Outside": "Humidity 0",
#     "Humidity Inside": "Humidity 1",
#     "Humidity Basement": "Humidity 2",
#     "Particle Count Outside": "Particle Count 0",
#     "Particle Count Inside": "Particle Count 1",
#     "Particle Count Basement": "Particle Count 2",
#     "O2 Outside": "O2 0",
#     "O2 Inside": "O2 1",
#     "O2 Basement": "O2 2",
# }

ACCENT = config.PALETTE[3]
OK_COLOR = config.PALETTE[2]
BAD_COLOR = config.PALETTE[1] # MAKE THIS YELLOW
ERROR_COLOR = config.PALETTE[3]

app = config.extension(url="fast_grid_template", template=None, intro_section=False)

app_html = "http://localhost:5006/visualize?theme=dark"

HEADER = [pn.Row(
        pn.layout.Spacer(sizing_mode="stretch_width"),
        pn.layout.VSpacer(width=4),
        height=86,
        sizing_mode="stretch_width",
    )]

if not STYLE in pn.config.raw_css:
    pn.config.raw_css.append(STYLE)

def get_average_data(sensor, selected_date):
    if(selected_date == None):
        selected_date = datetime.now().strftime('%Y-%m-%d')
    else:
        selected_date = selected_date.strftime('%Y-%m-%d')
    folder_path = 'SerialMonitorLogs'
    files = [f for f in os.listdir(folder_path) if f.endswith('.csv')]
    files = sorted(files)  # Make sure my list of files is sorted by date

    # Filter files that belong to the selected month
    current_month_files = [filename for filename in files if filename.startswith(selected_date[:7])]
    
    days = []
    avg_values = []
    
    for filename in current_month_files:
        file_path = os.path.join(folder_path, filename)
        try:
            df = pd.read_csv(file_path)
            avg_value = df[sensor].mean() 
            days.append(filename[:-4])
            avg_values.append(avg_value)

        except FileNotFoundError:
            print(f"File {file_path} not found.")
    data = {'Days': days, 'Daily Averages': avg_values}
    df_plot = pd.DataFrame(data)

    return df_plot

def get_current_value(card_name):
    #May want to implement a bidirectional dictionary for "instant" lookup
    card_name = next((name for name, t in SENSOR_TITLES.items() if t == card_name), None)
    current_date = datetime.now().strftime('%Y-%m-%d')
    file_path = f'{current_date}.csv'
    file_path = 'SerialMonitorLogs/' + file_path
    try:
        df = pd.read_csv(file_path)
        return df[card_name].iloc[-1]
    except FileNotFoundError:
        print(f"File {file_path} not found.")
        return None


def _create_callback(cards):
    async def update_card():
        index = 0
        for card in cards:
            card.value = get_current_value(card.name) #will come back to this index thing
            index += 1
    return update_card

def create_sensor_card(title, value, colors):
    if "Water" in title or "Temp" in title or "pH" in title:
        indicator = pn.indicators.LinearGauge(
            name=title,
            value=value,
            # bounds=(0, 100),  # Change these bounds as needed
            show_boundaries=True,
            height=650,
            width=300,
            colors=colors,
            css_classes=["pn-stats-card"],
        )
    elif "TDS" in title or "AQS" in title or "Humidity" in title or "O2" in title or "CO2" in title:
        indicator = pn.indicators.Dial(
            name=title,
            value=value,
            # annulus_width=10,
            # height=600,
            # width=300,
            # bounds=(0, 100),  # Change these bounds as needed
            show_boundaries=True,
            colors=colors,
            css_classes=["pn-stats-card"],
        )
    # elif "Temp" in title:
    #     indicator = pn.indicators.LinearGauge(
    #         name=title,
    #         value=value,
    #         bounds=(0, 100),
    #         show_boundaries=True,
    #         height=500,
    #         width=200,
    #         colors=colors,
    #         css_classes=["pn-stats-card"],
    #     )
    else:
        indicator = pn.indicators.Number(
            name=title,
            value=value,
            colors=colors,
            css_classes=["pn-stats-card"],
        )
    return indicator


# def create_sensor_card(title, value, colors):
#     indicator = pn.indicators.Number(
#         name=title,
#         value=value,
#         colors=colors,
#         css_classes=["pn-stats-card"],
#     )
#     return indicator

def create_app() -> pn.template.FastGridTemplate:
    """Returns an app"""

    #Initializes the template
    template = pn.template.FastGridTemplate(
        site="CITRIS",
        title="Sensor Streaming Dashboard",
        row_height=200,
        row_width=100,
        # sidebar=SIDEBAR,
        sidebar_footer=config.menu_fast_html(app_html="", accent=ACCENT),
        accent_base_color=ACCENT,
        header_background=ACCENT,
        header=HEADER,
        prevent_collision=True,
        save_layout=True,
    )

    template.main[0:1, 0:12] = pn.Column(
        pn.pane.Markdown("## This is where you will find all the info on the home\'s sensor system! Feel free to resize and move any and every icon to your liking."), sizing_mode="stretch_both"
    )

    # Sensor Cards display the current value of a sensor
    indicators = []

    default_value = 100

    color_thresholds = {
        SENSOR_NAMES[0]: [(100, OK_COLOR), (166, ERROR_COLOR)], # TDS  under 50 is bad, between 50 - 350 is good, over 350 bad
        SENSOR_NAMES[1]: [(100, OK_COLOR), (166, ERROR_COLOR)], # TDS
        SENSOR_NAMES[2]: [(100, ERROR_COLOR), (166, OK_COLOR)], # Water Height above 166 is good below 166 is bad
        SENSOR_NAMES[3]: [(100, ERROR_COLOR), (166, OK_COLOR)], # Water Height 
        SENSOR_NAMES[4]: [(100, ERROR_COLOR), (166, OK_COLOR)], # Water Height 
        SENSOR_NAMES[5]: [(100, ERROR_COLOR), (166, OK_COLOR)], # Water Height 
        SENSOR_NAMES[6]: [(66, OK_COLOR), (100, ERROR_COLOR)],  # Turbidity WE WILL COME BACK TO THIS
        SENSOR_NAMES[7]: [(66, OK_COLOR), (100, ERROR_COLOR)],  # Turbidity WE WILL COME BACK TO THIS
        SENSOR_NAMES[8]: [(66, OK_COLOR), (100, ERROR_COLOR)],  # pH
        SENSOR_NAMES[9]: [(66, OK_COLOR), (100, ERROR_COLOR)],  # pH
        SENSOR_NAMES[10]: [(66, OK_COLOR), (100, ERROR_COLOR)], # AQS
        SENSOR_NAMES[11]: [(66, OK_COLOR), (100, ERROR_COLOR)], # AQS
        SENSOR_NAMES[12]: [(66, OK_COLOR), (100, ERROR_COLOR)], # AQS
        SENSOR_NAMES[13]: [(66, OK_COLOR), (100, ERROR_COLOR)], # CO2
        SENSOR_NAMES[14]: [(66, OK_COLOR), (100, ERROR_COLOR)], # CO2
        SENSOR_NAMES[15]: [(66, OK_COLOR), (100, ERROR_COLOR)], # CO2
        SENSOR_NAMES[16]: [(66, OK_COLOR), (100, ERROR_COLOR)], # Temp
        SENSOR_NAMES[17]: [(66, OK_COLOR), (100, ERROR_COLOR)], # Temp
        SENSOR_NAMES[18]: [(66, OK_COLOR), (100, ERROR_COLOR)], # Temp
        SENSOR_NAMES[19]: [(66, OK_COLOR), (100, ERROR_COLOR)], # Humidity
        SENSOR_NAMES[20]: [(66, OK_COLOR), (100, ERROR_COLOR)], # Humidity
        SENSOR_NAMES[21]: [(66, OK_COLOR), (100, ERROR_COLOR)], # Humidity
        SENSOR_NAMES[22]: [(66, OK_COLOR), (100, ERROR_COLOR)], # Particle Count
        SENSOR_NAMES[23]: [(66, OK_COLOR), (100, ERROR_COLOR)], # Particle Count
        SENSOR_NAMES[24]: [(66, OK_COLOR), (100, ERROR_COLOR)], # Particle Count
        SENSOR_NAMES[25]: [(66, OK_COLOR), (100, ERROR_COLOR)], # O2
        SENSOR_NAMES[26]: [(66, OK_COLOR), (100, ERROR_COLOR)], # O2
        SENSOR_NAMES[27]: [(66, OK_COLOR), (100, ERROR_COLOR)]  # O2
    }

    sensor_format_map = {
        SENSOR_NAMES[0]: "{value}ppm",
        SENSOR_NAMES[1]: "{value}ppm",
        SENSOR_NAMES[2]: "{value}gal",
        SENSOR_NAMES[3]: "{value}gal",
        SENSOR_NAMES[4]: "{value}gal",
        SENSOR_NAMES[5]: "{value}gal",
        SENSOR_NAMES[6]: "{value}NTU",
        SENSOR_NAMES[7]: "{value}NTU",
        SENSOR_NAMES[8]: "{value}",
        SENSOR_NAMES[9]: "{value}",
        SENSOR_NAMES[10]: "{value}ppm",
        SENSOR_NAMES[11]: "{value}ppm",
        SENSOR_NAMES[12]: "{value}ppm", 
        SENSOR_NAMES[13]: "{value}ppm", 
        SENSOR_NAMES[14]: "{value}ppm",
        SENSOR_NAMES[15]: "{value}ppm",
        SENSOR_NAMES[16]: "{value}° f",
        SENSOR_NAMES[17]: "{value}° f",
        SENSOR_NAMES[18]: "{value}° f",
        SENSOR_NAMES[19]: "{value}%",
        SENSOR_NAMES[20]: "{value}%",
        SENSOR_NAMES[21]: "{value}%",
        SENSOR_NAMES[22]: "{value}ppm",
        SENSOR_NAMES[23]: "{value}ppm",
        SENSOR_NAMES[24]: "{value}ppm",
        SENSOR_NAMES[25]: "{value}%Vol",
        SENSOR_NAMES[26]: "{value}%Vol",
        SENSOR_NAMES[27]: "{value}%Vol"
    }

    row_col_pairs = [
        (1, 0), (1, 1), (1, 2), (1, 3), (1, 4), (1, 5),
        (2, 0), (2, 1), (2, 2), (2, 3), (2, 4), (2, 5),
        (3, 0), (3, 1), (3, 2), (3, 3), (3, 4), (3, 5),
        (4, 0), (4, 1), (4, 2), (4, 3), (4, 4), (4, 5),
        (5, 0), (5, 1), (5, 2), (5, 3), (5, 4), (5, 5)
    ]

    for (row, col), sensor_name in zip(row_col_pairs, SENSOR_NAMES):
        format_string = sensor_format_map[sensor_name] # Get format string for the sensor
        thresholds = color_thresholds.get(sensor_name, [])  # Get color thresholds for the sensor
        indicator = create_sensor_card(SENSOR_TITLES[sensor_name], default_value, thresholds)
        indicator.format = format_string
        template.main[row, 2 * col : 2 * col + 2] = indicator
        indicators.append(indicator)

    # Historical Averages

    template.main[6:7, 0:12] = pn.Column(
        pn.pane.Markdown("# Historical Averages"), sizing_mode="stretch_both"
    )

    # Get the first day of the current month
    current_date = date.today()
    first_day_of_month = current_date.replace(day=1)

    date_picker = pn.widgets.DatePicker(
        name='Choose Month',
        value=first_day_of_month,
        end=current_date.replace(day=1, month=current_date.month + 1)
    )

    # date_picker = pn.widgets.DatePicker(
    #     name='Date Picker', value=date.today(),
    # )

    select_sensor = pn.widgets.Select(
        name="Sensor", options=SENSOR_TITLES, value=SENSOR_TITLES[SENSOR_NAMES[0]] # REPLACE WITH TITLES!!!
    )

    plot_panel = pn.pane.HoloViews(sizing_mode="stretch_width")

    @pn.depends(select_sensor.param.value, date_picker.param.value, watch=True)  # type: ignore
    def _update_plot(*_):
        sensor_title = select_sensor.value
        sensor = next((name for name, t in SENSOR_TITLES.items() if t == sensor_title), None)
        selected_date = date_picker.value
        data = get_average_data(sensor, selected_date)

        # Convert the 'Days' column to datetime for proper axis formatting
        data['Days'] = pd.to_datetime(data['Days'])

        # Create a Holoviews Curve plot
        plot = hv.Curve(data, 'Days', 'Daily Averages').opts(
            title=sensor_title, color=ACCENT, responsive=True, height=800,
            fontscale=2,
            xaxis='top',  # Place the x-axis on top
            xrotation=45,  # Rotate x-axis labels for better visibility
            xlabel='',  # Remove x-axis label
            ylabel='Daily Averages',  # Set y-axis label
            show_grid=True,
        )

        plot_panel.object = plot



    _update_plot()

    template.main[7:13, 0:12] = pn.Column(
        pn.Tabs(
            pn.Row(select_sensor, name="By Sensor", margin=(10, 5, 25, 5)),
            pn.Row(select_sensor, date_picker, name="Past Months", margin=(10, 5, 25, 5)),
        ),
        plot_panel,
    )

    pn.state.add_periodic_callback(_create_callback(indicators), period=1000)
    # Maybe update the list of files in the callback function?

    return template

def serve():
    """Serves the app"""
    create_app().servable()

if __name__.startswith("bokeh"):
    serve()