from flask import Flask, render_template, request
import csv

app = Flask(__name__)

@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        if 'csv_file' not in request.files:
            return "No file uploaded", 400

        file = request.files['csv_file']
        if not file:
            return "No file uploaded", 400

        if file.filename == '':
            return "No file selected", 400

        if file:
            data = []
            stream = io.StringIO(file.stream.read().decode("UTF8"), newline=None)
            csv_input = csv.reader(stream)
            for row in csv_input:
                data.append(row)
            return render_template('display.html', data=data)
    return render_template('index.html')

if __name__ == '__main__':
    app.run(debug=True)

