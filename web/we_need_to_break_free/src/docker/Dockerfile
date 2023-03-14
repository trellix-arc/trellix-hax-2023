FROM python:latest

#Define working directory inside container
WORKDIR /app

#Install dependencies
COPY requirements.txt /app
RUN pip3 install -r requirements.txt

# copy the content of the local directory to the working directory
COPY ./src /app

EXPOSE 5000

#Run command on start
CMD [ "flask", "--app", "./app.py" ,"run", "--host=0.0.0.0"] 