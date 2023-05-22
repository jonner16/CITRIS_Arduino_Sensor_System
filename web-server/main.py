import os
import csv
from flask import Flask, render_template, jsonify

app = Flask(__name__)

# Route to display the HTML page that shows the CSV data
@app.route('/')
def display():
    return render_template('display.html')

# Route to return the CSV data in HTML format, without the surrounding HTML tags
@app.route('/csv_data')
def csv_data():
    # Get all CSV files in the assets directory
    csv_files = [f for f in os.listdir('assets') if f.endswith('.csv')]

    # Initialize a list to hold all the rows from all CSV files
    rows = []

    # Loop through each CSV file
    for csv_file in csv_files:
        # Open the CSV file and read its contents
        with open(os.path.join('assets', csv_file)) as f:
            csv_reader = csv.reader(f)
            # Skip the header row
            next(csv_reader)
            # Add each row to the list of rows
            rows.extend(csv_reader)

    # Get the header row from the first CSV file
    with open(os.path.join('assets', csv_files[0])) as f:
        csv_reader = csv.reader(f)
        headers = next(csv_reader)

    # Render just the table data as HTML, without the surrounding HTML tags
    table_html = render_template('table_data.html', headers=headers, rows=rows)

    return jsonify({'table_html': table_html})

if __name__ == '__main__':
    app.run(host="127.0.0.1", port=8080, debug=True)

