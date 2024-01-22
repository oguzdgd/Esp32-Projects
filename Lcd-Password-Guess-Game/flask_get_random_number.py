from flask import Flask, jsonify
import random

app = Flask(__name__)

@app.route('/get_random_number', methods=['GET'])
def get_random_number():
    random_number = random.randint(1000, 9999)
    return jsonify({'randomNumber': random_number})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
