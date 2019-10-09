import pandas
import numpy as np
import plotly.graph_objects as go


class Data():
    """A wrapper class to read and manipulate data from output produced by run.sh"""
    header_ = 11  # This should reflect the number of rows the header produces
    index_cols_ = ['list', 'size', 'update_rate', 'max_rqs', 'rq_threads', 'rq_rate',
                   'num_threads']

    def __init__(self, filename):
        self.df = pandas.read_csv(
            filename, sep=',', header=self.header_, engine='c')

    def plot_all_txs(self, x_axis, y_axis, match, filter_on, group_by):
        assert(len(match) == len(filter_on))
        data = self.df
        for i in range(len(filter_on)):
            data = data[data[filter_on[i]] == match[i]]
        unique = data[group_by].unique()
        layout = {'title': str(filter_on)+'='+str(match)}
        fig = go.Figure(layout=layout)
        symbol = 0
        for u in unique:
            d_ = data[data[group_by] == u]
            x_ = d_[x_axis].unique()
            y_ = d_[y_axis].to_numpy()
            marker_ = {'symbol': symbol, 'opacity': 1.0, 'size': 16}
            symbol += 1
            fig.add_trace(go.Scatter(x=x_, y=y_, mode='markers+lines',
                                     marker=marker_, name='rq_threads='+str(u)))
        return fig


if __name__ == '__main__':
    d = Data(filename='~/tmp/results/2ts-rwlock.csv')
    match = ('unsafe', 5000, 100, 100, 8)
    filter_on = ('list', 'size', 'update_rate', 'rq_rate', 'max_rqs')
    fig = d.plot_all_txs('num_threads', '#rq txs', match, filter_on, 'rq_threads')
    fig.show()
    fig = d.plot_all_txs('num_threads', '#update txs', match, filter_on, 'rq_threads')
    fig.show()
    fig = d.plot_all_txs('num_threads', '#txs', match, filter_on, 'rq_threads')
    fig.show()